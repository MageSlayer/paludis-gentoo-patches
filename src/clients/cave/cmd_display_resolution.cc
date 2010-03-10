/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010 Ciaran McCreesh
 *
 * This file is part of the Paludis package manager. Paludis is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License version 2, as published by the Free Software Foundation.
 *
 * Paludis is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "cmd_display_resolution.hh"
#include "cmd_resolve_cmdline.hh"
#include "exceptions.hh"
#include "command_command_line.hh"
#include "formats.hh"
#include "colour_formatter.hh"
#include "match_qpns.hh"
#include <paludis/args/do_help.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/util/system.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/join.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/options.hh>
#include <paludis/util/set.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/make_shared_copy.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/util/log.hh>
#include <paludis/resolver/resolutions.hh>
#include <paludis/resolver/reason.hh>
#include <paludis/resolver/sanitised_dependencies.hh>
#include <paludis/resolver/resolution.hh>
#include <paludis/resolver/decision.hh>
#include <paludis/resolver/constraint.hh>
#include <paludis/resolver/resolver.hh>
#include <paludis/resolver/resolvent.hh>
#include <paludis/resolver/destination.hh>
#include <paludis/resolver/unsuitable_candidates.hh>
#include <paludis/resolver/resolver_lists.hh>
#include <paludis/resolver/job.hh>
#include <paludis/resolver/jobs.hh>
#include <paludis/resolver/job_id.hh>
#include <paludis/resolver/required_confirmations.hh>
#include <paludis/package_id.hh>
#include <paludis/version_spec.hh>
#include <paludis/metadata_key.hh>
#include <paludis/choice.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/match_package.hh>
#include <paludis/repository.hh>
#include <paludis/package_dep_spec_properties.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/selection.hh>
#include <paludis/environment.hh>
#include <paludis/mask.hh>
#include <paludis/serialise.hh>

#include <set>
#include <iterator>
#include <iostream>
#include <cstdlib>

using namespace paludis;
using namespace cave;
using namespace paludis::resolver;

using std::cout;
using std::endl;

namespace
{
    struct DisplayResolutionCommandLine :
        CaveCommandCommandLine
    {
        ResolveCommandLineDisplayOptions display_options;
        ResolveCommandLineImportOptions import_options;

        DisplayResolutionCommandLine() :
            display_options(this),
            import_options(this)
        {
            add_environment_variable("PALUDIS_SERIALISED_RESOLUTION_FD",
                    "The file descriptor on which the serialised resolution can be found.");
        }

        virtual std::string app_name() const
        {
            return "cave display-resolution";
        }

        virtual std::string app_synopsis() const
        {
            return "Displays a dependency resolution created using 'cave resolve'.";
        }

        virtual std::string app_description() const
        {
            return "Displays a dependency resolution created using 'cave resolve'. Mostly for "
                "internal use; most users will not use this command directly.";
        }
    };

    std::string get_annotation(
            const std::tr1::shared_ptr<const MetadataSectionKey> & section,
            const std::string & name)
    {
        MetadataSectionKey::MetadataConstIterator i(section->find_metadata(name));
        if (i == section->end_metadata())
            return "";

        const MetadataValueKey<std::string> * value(
                simple_visitor_cast<const MetadataValueKey<std::string> >(**i));
        if (! value)
        {
            Log::get_instance()->message("cave.get_annotation.not_a_string", ll_warning, lc_context)
                << "Annotation '" << (*i)->raw_name() << "' not a string. This is probably a bug.";
            return "";
        }

        return value->value();
    }

    struct ReasonNameGetter
    {
        const bool verbose;
        const bool more_annotations;

        ReasonNameGetter(const bool v, const bool m) :
            verbose(v),
            more_annotations(m)
        {
        }

        std::pair<std::string, bool> annotate(
                const std::tr1::shared_ptr<const MetadataSectionKey> & key,
                const std::pair<std::string, bool> unannotated) const
        {
            if ((! key) || (! more_annotations))
                return unannotated;

            std::pair<std::string, bool> result(unannotated);

            std::string description_annotation(get_annotation(key, "description"));
            if (! description_annotation.empty())
            {
                result.first = result.first + ": \"" + description_annotation + "\"";
                result.second = true;
            }

            return result;
        }

        std::pair<std::string, bool> visit(const DependencyReason & r) const
        {
            if (r.sanitised_dependency().spec().if_block())
                return annotate(r.sanitised_dependency().spec().if_block()->annotations_key(),
                        std::make_pair(stringify(*r.sanitised_dependency().spec().if_block())
                            + " from " + (verbose ? stringify(*r.from_id()) : stringify(r.from_id()->name())),
                            true));
            else
            {
                if (verbose)
                {
                    std::string as;
                    if (! r.sanitised_dependency().original_specs_as_string().empty())
                        as = " (originally " + r.sanitised_dependency().original_specs_as_string() + ")";

                    return annotate(r.sanitised_dependency().spec().if_package()->annotations_key(),
                            std::make_pair(stringify(*r.sanitised_dependency().spec().if_package())
                                + " from " + stringify(*r.from_id()) + ", key '"
                                + r.sanitised_dependency().metadata_key_human_name() + "'"
                                + (r.sanitised_dependency().active_dependency_labels_as_string().empty() ? "" :
                                    ", labelled '" + r.sanitised_dependency().active_dependency_labels_as_string() + "'")
                                + as,
                                false));
                }
                else
                    return annotate(r.sanitised_dependency().spec().if_package()->annotations_key(),
                            std::make_pair(stringify(r.from_id()->name()), false));
            }
        }

        std::pair<std::string, bool> visit(const DependentReason & r) const
        {
            return std::make_pair("dependent upon " + stringify(*r.id_being_removed()), true);
        }

        std::pair<std::string, bool> visit(const TargetReason &) const
        {
            return std::make_pair("target", true);
        }

        std::pair<std::string, bool> visit(const SetReason & r) const
        {
            std::pair<std::string, bool> rr(r.reason_for_set()->accept_returning<std::pair<std::string, bool> >(*this));
            return std::make_pair(rr.first + " (" + stringify(r.set_name()) + ")", rr.second);
        }

        std::pair<std::string, bool> visit(const PresetReason & r) const
        {
            std::pair<std::string, bool> rr("", false);
            if (r.maybe_reason_for_preset())
                rr = r.maybe_reason_for_preset()->accept_returning<std::pair<std::string, bool> >(*this);

            rr.first = r.maybe_explanation() + (r.maybe_explanation().empty() || rr.first.empty() ? "" : " ")
                + rr.first;

            return rr;
        }
    };

    struct IDForDecisionOrNullVisitor
    {
        const std::tr1::shared_ptr<const PackageID> visit(const ExistingNoChangeDecision & d) const
        {
            return d.existing_id();
        }

        const std::tr1::shared_ptr<const PackageID> visit(const ChangesToMakeDecision & d) const
        {
            return d.origin_id();
        }

        const std::tr1::shared_ptr<const PackageID> visit(const UnableToMakeDecision &) const
        {
            return make_null_shared_ptr();
        }

        const std::tr1::shared_ptr<const PackageID> visit(const RemoveDecision &) const
        {
            return make_null_shared_ptr();
        }

        const std::tr1::shared_ptr<const PackageID> visit(const NothingNoChangeDecision &) const
        {
            return make_null_shared_ptr();
        }
    };

    const std::tr1::shared_ptr<const PackageID> id_for_decision_or_null(const Decision & d)
    {
        return d.accept_returning<std::tr1::shared_ptr<const PackageID> >(IDForDecisionOrNullVisitor());
    }

    bool decision_matches_spec(
            const std::tr1::shared_ptr<Environment> & env,
            const Resolvent & resolvent,
            const Decision & decision,
            const PackageDepSpec & spec)
    {
        const std::tr1::shared_ptr<const PackageID> maybe_id(id_for_decision_or_null(decision));
        if (maybe_id)
            return match_package(*env, spec, *maybe_id, MatchPackageOptions());
        else
        {
            /* could also match slot here too */
            return match_qpns(*env, spec, resolvent.package());
        }
    }

    const std::string constraint_as_string(const Constraint & c)
    {
        std::stringstream result;
        result << c.spec();

        switch (c.use_existing())
        {
            case ue_if_same:
                result << ", use existing if same";
                break;
            case ue_never:
                result << ", never using existing";
                break;
            case ue_only_if_transient:
                result << ", using existing only if transient";
                break;
            case ue_if_same_version:
                result << ", use existing if same version";
                break;
            case ue_if_possible:
                result << ", use existing if possible";
                break;

            case last_ue:
                break;
        }

        switch (c.destination_type())
        {
            case dt_install_to_slash:
                result << ", installing to /";
                break;

            case dt_create_binary:
                result << ", creating a binary";
                break;

            case last_dt:
                break;
        }

        return result.str();
    }

    void display_explanation_constraints(const Constraints & constraints)
    {
        cout << "    The following constraints were in action:" << endl;
        for (Constraints::ConstIterator c(constraints.begin()), c_end(constraints.end()) ;
                c != c_end ; ++c)
        {
            cout << "      * " << constraint_as_string(**c) << endl;
            cout << "        Because of ";
            ReasonNameGetter g(true, true);
            cout << (*c)->reason()->accept_returning<std::pair<std::string, bool> >(g).first;
            cout << endl;
        }
    }

    struct DisplayExplanationDecisionVisitor
    {
        void visit(const ExistingNoChangeDecision & d) const
        {
            cout << "    The decision made was:" << endl;
            cout << "        Use existing ID " << *d.existing_id() << endl;
        }

        void visit(const RemoveDecision & d) const
        {
            cout << "    The decision made was:" << endl;
            cout << "        Remove existing IDs" << endl;
            for (PackageIDSequence::ConstIterator i(d.ids()->begin()), i_end(d.ids()->end()) ;
                    i != i_end ; ++i)
                cout << "            Remove " << **i << endl;
        }

        void visit(const NothingNoChangeDecision &) const
        {
            cout << "    The decision made was:" << endl;
            cout << "        Do not do anything" << endl;
        }

        void visit(const ChangesToMakeDecision & d) const
        {
            cout << "    The decision made was:" << endl;
            cout << "        Use origin ID " << *d.origin_id() << endl;
            cout << "        Install to repository " << d.destination()->repository() << endl;
            for (PackageIDSequence::ConstIterator i(d.destination()->replacing()->begin()), i_end(d.destination()->replacing()->end()) ;
                    i != i_end ; ++i)
                cout << "            Replacing " << **i << endl;
        }

        void visit(const UnableToMakeDecision &) const
        {
            cout << "    No decision could be made" << endl;
        }
    };

    void display_explanation_decision(const Decision & decision)
    {
        decision.accept(DisplayExplanationDecisionVisitor());
    }

    void display_explanations(
            const std::tr1::shared_ptr<Environment> & env,
            const ResolverLists & lists,
            const DisplayResolutionCommandLine & cmdline)
    {
        Context context("When displaying explanations:");

        if (cmdline.display_options.a_explain.begin_args() == cmdline.display_options.a_explain.end_args())
            return;

        cout << "Explaining requested decisions:" << endl << endl;

        for (args::StringSetArg::ConstIterator i(cmdline.display_options.a_explain.begin_args()),
                i_end(cmdline.display_options.a_explain.end_args()) ;
                i != i_end ; ++i)
        {
            bool any(false);
            PackageDepSpec spec(parse_user_package_dep_spec(*i, env.get(), UserPackageDepSpecOptions() + updso_allow_wildcards));
            for (Resolutions::ConstIterator r(lists.all_resolutions()->begin()), r_end(lists.all_resolutions()->end()) ;
                    r != r_end ; ++r)
            {
                if (! decision_matches_spec(env, (*r)->resolvent(), *(*r)->decision(), spec))
                    continue;

                any = true;

                cout << "For " << (*r)->resolvent() << ":" << endl;

                display_explanation_constraints(*(*r)->constraints());
                display_explanation_decision(*(*r)->decision());
            }

            if (! any)
                throw args::DoHelp("There is nothing matching '" + *i + "' in the resolution set.");
        }
    }

    void display_one_description(
            const std::tr1::shared_ptr<Environment> &,
            const DisplayResolutionCommandLine & cmdline,
            const std::tr1::shared_ptr<const PackageID> & id,
            const bool is_new)
    {
        if (id->short_description_key())
        {
            bool show(false);
            if (cmdline.display_options.a_show_descriptions.argument() == "none")
                show = false;
            else if (cmdline.display_options.a_show_descriptions.argument() == "new")
                show = is_new;
            else if (cmdline.display_options.a_show_descriptions.argument() == "all")
                show = true;
            else
                throw args::DoHelp("Don't understand argument '"
                        + cmdline.display_options.a_show_descriptions.argument() + "' to '--"
                        + cmdline.display_options.a_show_descriptions.long_name() + "'");

            if (show)
                cout << "    \"" << id->short_description_key()->value() << "\"" << endl;
        }
    }

    void display_choices(
            const std::tr1::shared_ptr<Environment> &,
            const DisplayResolutionCommandLine &,
            const std::tr1::shared_ptr<const PackageID> & id,
            const std::tr1::shared_ptr<const PackageID> & old_id
            )
    {
        if (! id->choices_key())
            return;

        ColourFormatter formatter(0);

        std::tr1::shared_ptr<const Choices> old_choices;
        if (old_id && old_id->choices_key())
            old_choices = old_id->choices_key()->value();

        bool non_blank_prefix(false);
        std::string s;
        for (Choices::ConstIterator k(id->choices_key()->value()->begin()),
                k_end(id->choices_key()->value()->end()) ;
                k != k_end ; ++k)
        {
            if ((*k)->hidden())
                continue;

            bool shown_prefix(false);
            for (Choice::ConstIterator i((*k)->begin()), i_end((*k)->end()) ;
                    i != i_end ; ++i)
            {
                if (! (*i)->explicitly_listed())
                    continue;

                if (! shown_prefix)
                {
                    if (non_blank_prefix || ! (*k)->show_with_no_prefix())
                    {
                        shown_prefix = true;
                        if (! s.empty())
                            s.append(" ");
                        s.append((*k)->raw_name() + ":");
                    }
                }

                if (! s.empty())
                    s.append(" ");

                std::string t;
                if ((*i)->enabled())
                {
                    if ((*i)->locked())
                        t = formatter.format(**i, format::Forced());
                    else
                        t = formatter.format(**i, format::Enabled());
                }
                else
                {
                    if ((*i)->locked())
                        t = formatter.format(**i, format::Masked());
                    else
                        t = formatter.format(**i, format::Disabled());
                }

                bool changed(false), added(false);
                if ((*k)->consider_added_or_changed())
                {
                    if (old_choices)
                    {
                        std::tr1::shared_ptr<const ChoiceValue> old_choice(
                                old_choices->find_by_name_with_prefix((*i)->name_with_prefix()));
                        if (! old_choice)
                            added = true;
                        else if (old_choice->enabled() != (*i)->enabled())
                            changed = true;
                    }
                    else
                        added = true;
                }

                if (changed)
                {
                    t = formatter.decorate(**i, t, format::Changed());
                }
                else if (added)
                {
                    if (old_id)
                        t = formatter.decorate(**i, t, format::Added());
                }

                s.append(t);
            }
        }

        if (s.empty())
            return;

        cout << "    " << s << endl;
    }

    void display_reasons(
            const std::tr1::shared_ptr<const Resolution> & resolution,
            const bool more_annotations
            )
    {
        std::set<std::string> reasons, special_reasons;
        for (Constraints::ConstIterator c(resolution->constraints()->begin()),
                c_end(resolution->constraints()->end()) ;
                c != c_end ; ++c)
        {
            ReasonNameGetter g(false, more_annotations);
            std::pair<std::string, bool> r((*c)->reason()->accept_returning<std::pair<std::string, bool> >(g));
            if (r.first.empty())
                continue;

            if (r.second)
                special_reasons.insert(r.first);
            else
                reasons.insert(r.first);
        }

        for (std::set<std::string>::const_iterator r(special_reasons.begin()), r_end(special_reasons.end()) ;
                r != r_end ; ++r)
            reasons.erase(*r);

        if (reasons.empty() && special_reasons.empty())
            return;

        cout << "    Reasons: ";

        int n_shown(0);
        for (std::set<std::string>::const_iterator r(special_reasons.begin()), r_end(special_reasons.end()) ;
                r != r_end ; ++r)
        {
            if (++n_shown != 1)
                cout << ", ";
            cout << c::bold_yellow() << *r << c::normal();
        }

        int n_remaining(reasons.size());
        for (std::set<std::string>::const_iterator r(reasons.begin()), r_end(reasons.end()) ;
                r != r_end ; ++r)
        {
            if (n_shown >= 3 && n_remaining > 1)
            {
                cout << ", " << n_remaining << " more";
                break;
            }

            --n_remaining;
            if (++n_shown != 1)
                cout << ", ";
            cout << c::yellow() << *r << c::normal();
        }

        cout << endl;
    }

    struct DisplayConfirmationVisitor
    {
        std::string visit(const DowngradeConfirmation &) const
        {
            return "--permit-downgrade";
        }

        std::string visit(const NotBestConfirmation &) const
        {
            return "--permit-any-version";
        }
    };

    std::string stringify_confirmation(const RequiredConfirmation & c)
    {
        return c.accept_returning<std::string>(DisplayConfirmationVisitor());
    }

    void display_confirmations(
            const Job & job)
    {
        const std::tr1::shared_ptr<const RequiredConfirmations> r(job.required_confirmations());
        if (! r->empty())
            cout << c::bold_red() << "    Cannot proceed without: " << c::normal() <<
                join(indirect_iterator(r->begin()), indirect_iterator(r->end()), ", ", stringify_confirmation) << endl;
    }

    void display_one_installish(
            const std::tr1::shared_ptr<Environment> & env,
            const DisplayResolutionCommandLine & cmdline,
            const ChangesToMakeDecision & decision,
            const std::tr1::shared_ptr<const Resolution> & resolution,
            const SimpleInstallJob & job,
            const bool more_annotations,
            const bool confirmations)
    {
        std::string x("   ");
        if (! decision.best())
            x[0] = '-';

        do
        {
            switch (decision.change_type())
            {
                case ct_new:
                    cout << "n" << x << c::bold_blue();
                    continue;
                case ct_slot_new:
                    cout << "s" << x << c::bold_blue();
                    continue;
                case ct_upgrade:
                    cout << "u" << x << c::blue();
                    continue;
                case ct_reinstall:
                    cout << "r" << x << c::yellow();
                    continue;
                case ct_downgrade:
                    cout << "d" << x << c::bold_yellow();
                    continue;
                case last_ct:
                    break;
            }
            throw InternalError(PALUDIS_HERE, "bad change_type. huh?");
        }
        while (false);

        cout << decision.origin_id()->canonical_form(idcf_no_version);

        if (! decision.best())
            cout << c::bold_yellow() << " (not the best version)" << c::normal();

        cout << c::normal() << " " << decision.origin_id()->canonical_form(idcf_version) <<
            " to " << decision.destination()->repository();

        if (! decision.destination()->replacing()->empty())
        {
            cout << " replacing ";
            bool first(true);
            for (PackageIDSequence::ConstIterator i(decision.destination()->replacing()->begin()),
                    i_end(decision.destination()->replacing()->end()) ;
                    i != i_end ; ++i)
            {
                if (! first)
                    cout << ", ";
                first = false;

                if ((*i)->name() == decision.origin_id()->name())
                    cout << (*i)->canonical_form(idcf_version);
                else
                    cout << (*i)->canonical_form(idcf_full);
            }
        }
        cout << endl;

        std::tr1::shared_ptr<const PackageID> old_id;
        if (! decision.destination()->replacing()->empty())
            old_id = *decision.destination()->replacing()->begin();

        display_one_description(env, cmdline, decision.origin_id(), ! old_id);
        display_choices(env, cmdline, decision.origin_id(), old_id);
        display_reasons(resolution, more_annotations);
        if (confirmations)
            display_confirmations(job);
    }

    void display_one_install(
            const std::tr1::shared_ptr<Environment> & env,
            const DisplayResolutionCommandLine & cmdline,
            const SimpleInstallJob & job,
            const bool more_annotations,
            const bool confirmations)
    {
        display_one_installish(env, cmdline, *job.changes_to_make_decision(), job.resolution(),
                job, more_annotations, confirmations);
    }

    void display_one_uninstall(
            const std::tr1::shared_ptr<Environment> &,
            const DisplayResolutionCommandLine &,
            const UninstallJob & job,
            const bool more_annotations,
            const bool confirmations)
    {
        cout << "<   " << c::bold_green() << job.resolution()->resolvent().package() << c::normal() << " ";

        bool first(true);
        for (PackageIDSequence::ConstIterator i(job.remove_decision()->ids()->begin()),
                i_end(job.remove_decision()->ids()->end()) ;
                i != i_end ; ++i)
        {
            if (! first)
                cout << ", ";
            first = false;

            cout << (*i)->canonical_form(idcf_no_name);
        }

        cout << endl;
        display_reasons(job.resolution(), more_annotations);
        if (confirmations)
            display_confirmations(job);
    }

    void display_special_job_decision(
            const std::tr1::shared_ptr<Environment> &,
            const DisplayResolutionCommandLine &,
            const std::string & job_name,
            const std::tr1::shared_ptr<const PackageID> & id)
    {
        cout << "-   " << job_name << " " << *id << endl;
    }

    void display_unable_to_make_decision(
            const std::tr1::shared_ptr<Environment> & env,
            const std::tr1::shared_ptr<const Resolution> & resolution,
            const UnableToMakeDecision & d)
    {
        cout << "!   " << c::bold_red() << resolution->resolvent() << c::normal() << endl;
        display_reasons(resolution, true);

        if (d.unsuitable_candidates()->empty())
            return;

        cout << "    Unsuitable candidates:" << endl;
        for (UnsuitableCandidates::ConstIterator u(d.unsuitable_candidates()->begin()),
                u_end(d.unsuitable_candidates()->end()) ;
                u != u_end ; ++u)
        {
            cout << "      * " << *u->package_id() << endl;
            for (PackageID::MasksConstIterator m(u->package_id()->begin_masks()),
                    m_end(u->package_id()->end_masks()) ;
                    m != m_end ; ++m)
                cout << "        Masked by " << (*m)->description() << endl;

            std::set<std::string> duplicates;
            for (Constraints::ConstIterator c(u->unmet_constraints()->begin()),
                    c_end(u->unmet_constraints()->end()) ;
                    c != c_end ; ++c)
            {
                ReasonNameGetter g(false, true);
                std::string s(constraint_as_string(**c) + " from " +
                        (*c)->reason()->accept_returning<std::pair<std::string, bool> >(g).first);

                if (! duplicates.insert(s).second)
                    continue;

                cout << "        Did not meet " << s << endl;

                if ((*c)->spec().if_package() && (*c)->spec().if_package()->additional_requirements_ptr() &&
                        (! match_package(*env, *(*c)->spec().if_package(), *u->package_id(), MatchPackageOptions())) &&
                        match_package(*env, *(*c)->spec().if_package(), *u->package_id(), MatchPackageOptions() + mpo_ignore_additional_requirements))
                {
                    for (AdditionalPackageDepSpecRequirements::ConstIterator a((*c)->spec().if_package()->additional_requirements_ptr()->begin()),
                            a_end((*c)->spec().if_package()->additional_requirements_ptr()->end()) ;
                            a != a_end ; ++a)
                    {
                        const std::pair<bool, std::string> p((*a)->requirement_met(env.get(), *u->package_id()));
                        if (p.first)
                            continue;

                        cout << "            " << p.second << endl;
                    }
                }
            }
        }
    }

    struct ShowJobsDisplayer
    {
        const std::tr1::shared_ptr<Environment> env;
        const DisplayResolutionCommandLine & cmdline;
        const ResolverLists & lists;
        const bool all;
        const bool more_annotations;
        const bool confirmations;

        ShowJobsDisplayer(
                const std::tr1::shared_ptr<Environment> & e,
                const DisplayResolutionCommandLine & c,
                const ResolverLists & l,
                const bool a,
                const bool n,
                const bool f
                ) :
            env(e),
            cmdline(c),
            lists(l),
            all(a),
            more_annotations(n),
            confirmations(f)
        {
        }

        bool visit(const SimpleInstallJob & job)
        {
            display_one_install(env, cmdline, job, more_annotations, confirmations);
            return true;
        }

        bool visit(const UninstallJob & job)
        {
            display_one_uninstall(env, cmdline, job, more_annotations, confirmations);
            return true;
        }

        bool visit(const UsableJob & job, const bool indent = false)
        {
            if (! all)
                return false;

            if (indent)
                cout << "    ";
            cout << "-   " << c::bold_normal() << job.resolution()->resolvent() << c::normal() << " now usable" << endl;
            return true;
        }

        bool visit(const UsableGroupJob & job)
        {
            if (! all)
                return false;

            cout << "-   " << c::bold_normal() << "group of jobs now usable:" << c::normal() << endl;
            for (JobIDSequence::ConstIterator i(job.job_ids()->begin()), i_end(job.job_ids()->end()) ;
                    i != i_end ; ++i)
                visit(*lists.jobs()->fetch_as<UsableJob>(*i), true);
            return true;
        }

        bool visit(const FetchJob & job)
        {
            if (! all)
                return false;

            cout << "-   " << c::bold_normal() << job.resolution()->resolvent() << c::normal() << " fetch" << endl;
            return true;
        }

        bool visit(const ErrorJob &)
        {
            return false;
        }
    };

    struct JobNameVisitor
    {
        const std::string visit(const SimpleInstallJob & j) const
        {
            return "install " + stringify(*j.changes_to_make_decision()->origin_id());
        }

        const std::string visit(const UninstallJob & j) const
        {
            return "uninstall " + stringify(j.resolution()->resolvent());
        }

        const std::string visit(const UsableJob & j) const
        {
            return "usable " + stringify(j.resolution()->resolvent());
        }

        const std::string visit(const ErrorJob &) const
        {
            return "error";
        }

        const std::string visit(const FetchJob & j) const
        {
            return "fetch " + stringify(*j.changes_to_make_decision()->origin_id());
        }

        const std::string visit(const UsableGroupJob & j) const
        {
            return "usable (" + stringify(std::distance(j.job_ids()->begin(), j.job_ids()->end())) + ")";
        }
    };

    void display_jobs(
            const std::tr1::shared_ptr<Environment> & env,
            const ResolverLists & lists,
            const DisplayResolutionCommandLine & cmdline)
    {
        Context context("When displaying jobs:");

        cout << "These are the actions I will take, in order:" << endl << endl;

        bool any(false);
        for (JobIDSequence::ConstIterator i(lists.taken_job_ids()->begin()),
                i_end(lists.taken_job_ids()->end()) ;
                i != i_end ; ++i)
        {
            const std::tr1::shared_ptr<const Job> job(lists.jobs()->fetch(*i));
            ShowJobsDisplayer d(env, cmdline, lists, cmdline.display_options.a_show_all_jobs.specified() ||
                    ! job->used_existing_packages_when_ordering()->empty(), false, false);
            any |= job->accept_returning<bool>(d);

            if (! job->used_existing_packages_when_ordering()->empty())
            {
                std::set<std::string> missing;
                for (JobIDSequence::ConstIterator j(job->used_existing_packages_when_ordering()->begin()),
                        j_end(job->used_existing_packages_when_ordering()->end()) ;
                        j != j_end ; ++j)
                    missing.insert(lists.jobs()->fetch(*j)->accept_returning<std::string>(JobNameVisitor()));
                cout << "    " << c::bold_normal() << "Couldn't do before: " << c::normal()
                    << join(missing.begin(), missing.end(), ", ") << endl;
            }
        }

        if (! any)
            cout << "(nothing to do)" << endl;

        cout << endl;
    }

    void display_one_error(
            const std::tr1::shared_ptr<Environment> & env,
            const DisplayResolutionCommandLine &,
            const std::tr1::shared_ptr<const Resolution> & resolution,
            const ErrorJob & j)
    {
        display_unable_to_make_decision(env, resolution, *j.unable_to_make_decision());
    }

    void display_untaken(
            const std::tr1::shared_ptr<Environment> & env,
            const ResolverLists & lists,
            const DisplayResolutionCommandLine & cmdline)
    {
        Context context("When displaying untaken jobs:");

        if (lists.untaken_job_ids()->empty() && lists.untaken_error_job_ids()->empty())
            return;

        cout << "I did not take the following:" << endl << endl;

        for (JobIDSequence::ConstIterator i(lists.untaken_job_ids()->begin()),
                i_end(lists.untaken_job_ids()->end()) ;
                i != i_end ; ++i)
        {
            const std::tr1::shared_ptr<const Job> job(lists.jobs()->fetch(*i));
            ShowJobsDisplayer d(env, cmdline, lists, cmdline.display_options.a_show_all_jobs.specified() ||
                    ! job->used_existing_packages_when_ordering()->empty(), true, false);
            if (! job->accept_returning<bool>(d))
                throw InternalError(PALUDIS_HERE, "why didn't we get true?");
        }

        for (JobIDSequence::ConstIterator i(lists.untaken_error_job_ids()->begin()),
                i_end(lists.untaken_error_job_ids()->end()) ;
                i != i_end ; ++i)
        {
            const std::tr1::shared_ptr<const ErrorJob> job(lists.jobs()->fetch_as<ErrorJob>(*i));
            display_one_error(env, cmdline, job->resolution(), *job);
        }

        cout << endl;
    }

    void display_confirmation_jobs(
            const std::tr1::shared_ptr<Environment> & env,
            const ResolverLists & lists,
            const DisplayResolutionCommandLine & cmdline)
    {
        Context context("When displaying jobs requiring confirmation:");

        if (lists.job_ids_needing_confirmation()->empty())
            return;

        cout << "I cannot proceed without being permitted to do the following:" << endl << endl;

        for (JobIDSequence::ConstIterator i(lists.job_ids_needing_confirmation()->begin()),
                i_end(lists.job_ids_needing_confirmation()->end()) ;
                i != i_end ; ++i)
        {
            const std::tr1::shared_ptr<const Job> job(lists.jobs()->fetch(*i));
            ShowJobsDisplayer d(env, cmdline, lists, cmdline.display_options.a_show_all_jobs.specified() ||
                    ! job->used_existing_packages_when_ordering()->empty(), true, true);
            if (! job->accept_returning<bool>(d))
                throw InternalError(PALUDIS_HERE, "why didn't we get true?");
        }

        cout << endl;
    }

    void display_errors(
            const std::tr1::shared_ptr<Environment> & env,
            const ResolverLists & lists,
            const DisplayResolutionCommandLine & cmdline)
    {
        Context context("When displaying errors:");

        if (lists.taken_error_job_ids()->empty())
            return;

        cout << "I encountered the following errors:" << endl << endl;

        for (JobIDSequence::ConstIterator i(lists.taken_error_job_ids()->begin()),
                i_end(lists.taken_error_job_ids()->end()) ;
                i != i_end ; ++i)
        {
            const std::tr1::shared_ptr<const ErrorJob> job(lists.jobs()->fetch_as<ErrorJob>(*i));
            display_one_error(env, cmdline, job->resolution(), *job);
        }

        cout << endl;
    }
}

bool
DisplayResolutionCommand::important() const
{
    return false;
}

int
DisplayResolutionCommand::run(
        const std::tr1::shared_ptr<Environment> & env,
        const std::tr1::shared_ptr<const Sequence<std::string > > & args,
        const std::tr1::shared_ptr<const ResolverLists> & maybe_lists
        )
{
    DisplayResolutionCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_DISPLAY_RESOLUTION_OPTIONS", "CAVE_DISPLAY_RESOLUTION_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    cmdline.import_options.apply(env);

    std::tr1::shared_ptr<const ResolverLists> lists(maybe_lists);
    if (! lists)
    {
        if (getenv_with_default("PALUDIS_SERIALISED_RESOLUTION_FD", "").empty())
            throw args::DoHelp("PALUDIS_SERIALISED_RESOLUTION_FD must be provided");

        int fd(destringify<int>(getenv_with_default("PALUDIS_SERIALISED_RESOLUTION_FD", "")));
        SafeIFStream deser_stream(fd);
        const std::string deser_str((std::istreambuf_iterator<char>(deser_stream)), std::istreambuf_iterator<char>());
        Deserialiser deserialiser(env.get(), deser_str);
        Deserialisation deserialisation("ResolverLists", deserialiser);
        lists = make_shared_copy(ResolverLists::deserialise(deserialisation));
    }

    display_jobs(env, *lists, cmdline);
    display_untaken(env, *lists, cmdline);
    display_confirmation_jobs(env, *lists, cmdline);
    display_errors(env, *lists, cmdline);
    display_explanations(env, *lists, cmdline);

    return 0;
}

int
DisplayResolutionCommand::run(
        const std::tr1::shared_ptr<Environment> & env,
        const std::tr1::shared_ptr<const Sequence<std::string > > & args)
{
    return run(env, args, make_null_shared_ptr());
}

std::tr1::shared_ptr<args::ArgsHandler>
DisplayResolutionCommand::make_doc_cmdline()
{
    return make_shared_ptr(new DisplayResolutionCommandLine);
}

