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
#include <paludis/util/pretty_print.hh>
#include <paludis/resolver/resolutions_by_resolvent.hh>
#include <paludis/resolver/reason.hh>
#include <paludis/resolver/sanitised_dependencies.hh>
#include <paludis/resolver/resolution.hh>
#include <paludis/resolver/decision.hh>
#include <paludis/resolver/constraint.hh>
#include <paludis/resolver/resolver.hh>
#include <paludis/resolver/resolvent.hh>
#include <paludis/resolver/destination.hh>
#include <paludis/resolver/unsuitable_candidates.hh>
#include <paludis/resolver/decisions.hh>
#include <paludis/resolver/required_confirmations.hh>
#include <paludis/resolver/orderer_notes.hh>
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
#include <iomanip>
#include <cstdlib>
#include <map>

using namespace paludis;
using namespace cave;
using namespace paludis::resolver;

using std::cout;
using std::endl;

typedef std::map<ChoiceNameWithPrefix, std::tr1::shared_ptr<PackageIDSequence> > ChoiceValuesToExplain;
typedef std::map<std::string, ChoiceValuesToExplain> ChoicesToExplain;

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

        const std::tr1::shared_ptr<const PackageID> visit(const BreakDecision & d) const
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

        if (c.untaken())
            result << " (untaken)";

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
            if (d.taken())
                cout << "        Use existing ID " << *d.existing_id() << endl;
            else
                cout << "        Do not take existing ID " << *d.existing_id() << endl;
        }

        void visit(const RemoveDecision & d) const
        {
            cout << "    The decision made was:" << endl;
            if (d.taken())
                cout << "        Remove existing IDs" << endl;
            else
                cout << "        No not remove existing IDs" << endl;
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
            if (d.taken())
                cout << "    The decision made was:" << endl;
            else
                cout << "    The decision made was not to:" << endl;
            cout << "        Use origin ID " << *d.origin_id() << endl;
            cout << "        Install to repository " << d.destination()->repository() << endl;
            for (PackageIDSequence::ConstIterator i(d.destination()->replacing()->begin()), i_end(d.destination()->replacing()->end()) ;
                    i != i_end ; ++i)
                cout << "            Replacing " << **i << endl;
        }

        void visit(const UnableToMakeDecision & d) const
        {
            if (d.taken())
                cout << "    No decision could be made" << endl;
            else
                cout << "    No decision could be made, but none was necessary" << endl;
        }

        void visit(const BreakDecision & d) const
        {
            if (d.taken())
                cout << "    The decision made was to break " << *d.existing_id() << endl;
            else
                cout << "    The decision made would be to break " << *d.existing_id() << endl;
        }
    };

    void display_explanation_decision(const Decision & decision)
    {
        decision.accept(DisplayExplanationDecisionVisitor());
    }

    void display_explanations(
            const std::tr1::shared_ptr<Environment> & env,
            const std::tr1::shared_ptr<const Resolved> & resolved,
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
            for (ResolutionsByResolvent::ConstIterator r(resolved->resolutions_by_resolvent()->begin()),
                    r_end(resolved->resolutions_by_resolvent()->end()) ;
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
            const DisplayResolutionCommandLine & cmdline,
            const std::tr1::shared_ptr<const PackageID> & id,
            const std::tr1::shared_ptr<const PackageID> & old_id,
            ChoicesToExplain & choices_to_explain
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

                bool show_description;
                if (cmdline.display_options.a_show_option_descriptions.argument() == "none")
                    show_description = false;
                else if (cmdline.display_options.a_show_option_descriptions.argument() == "new")
                    show_description = ! old_id;
                else if (cmdline.display_options.a_show_option_descriptions.argument() == "changed")
                    show_description = added || ! old_id;
                else if (cmdline.display_options.a_show_option_descriptions.argument() == "all")
                    show_description = true;
                else
                    throw args::DoHelp("Don't understand argument '"
                            + cmdline.display_options.a_show_option_descriptions.argument() + "' to '--"
                            + cmdline.display_options.a_show_option_descriptions.long_name() + "'");

                if (show_description)
                    choices_to_explain.insert(std::make_pair((*k)->human_name(),
                                ChoiceValuesToExplain())).first->second.insert(std::make_pair(
                                (*i)->name_with_prefix(), make_shared_ptr(
                                    new PackageIDSequence))).first->second->push_back(id);
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
            return "--permit-old-version";
        }

        std::string visit(const BreakConfirmation &) const
        {
            return "--uninstalls-may-break or --remove-if-dependent";
        }
    };

    std::string stringify_confirmation(const RequiredConfirmation & c)
    {
        return c.accept_returning<std::string>(DisplayConfirmationVisitor());
    }

    void display_confirmations(
            const ConfirmableDecision & decision)
    {
        const std::tr1::shared_ptr<const RequiredConfirmations> r(decision.required_confirmations_if_any());
        if (r && ! r->empty())
            cout << c::bold_red() << "    Cannot proceed without: " << c::normal() <<
                join(indirect_iterator(r->begin()), indirect_iterator(r->end()), ", ", stringify_confirmation) << endl;
    }

    void display_one_installish(
            const std::tr1::shared_ptr<Environment> & env,
            const DisplayResolutionCommandLine & cmdline,
            const ChangesToMakeDecision & decision,
            const std::tr1::shared_ptr<const Resolution> & resolution,
            const bool more_annotations,
            const bool confirmations,
            const bool untaken,
            const std::string & notes,
            ChoicesToExplain & choices_to_explain)
    {
        std::string x("X");
        if (! decision.best())
            x = "-" + x;
        if (untaken)
            x = "(" + x + ")";

        if (x.length() < 4)
            x = x + std::string(4 - x.length(), ' ');

        do
        {
            switch (decision.change_type())
            {
                case ct_new:
                    cout << x.replace(x.find('X'), 1, "n") << c::bold_blue();
                    continue;
                case ct_slot_new:
                    cout << x.replace(x.find('X'), 1, "s") << c::bold_blue();
                    continue;
                case ct_upgrade:
                    cout << x.replace(x.find('X'), 1, "u") << c::blue();
                    continue;
                case ct_reinstall:
                    cout << x.replace(x.find('X'), 1, "r") << c::yellow();
                    continue;
                case ct_downgrade:
                    cout << x.replace(x.find('X'), 1, "d") << c::bold_yellow();
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
        display_choices(env, cmdline, decision.origin_id(), old_id, choices_to_explain);
        display_reasons(resolution, more_annotations);
        if (confirmations)
            display_confirmations(decision);
        if (! notes.empty())
            cout << "    " << c::bold_normal() << notes << c::normal() << endl;
    }

    void display_one_uninstall(
            const std::tr1::shared_ptr<Environment> &,
            const DisplayResolutionCommandLine &,
            const std::tr1::shared_ptr<const Resolution> & resolution,
            const RemoveDecision & decision,
            const bool more_annotations,
            const bool untaken)
    {
        if (untaken)
            cout << "(<) " << c::bold_green() << decision.resolvent().package() << c::normal() << " ";
        else
            cout << "<   " << c::bold_green() << decision.resolvent().package() << c::normal() << " ";

        bool first(true);
        for (PackageIDSequence::ConstIterator i(decision.ids()->begin()),
                i_end(decision.ids()->end()) ;
                i != i_end ; ++i)
        {
            if (! first)
                cout << ", ";
            first = false;

            cout << (*i)->canonical_form(idcf_no_name);
        }

        cout << endl;
        display_reasons(resolution, more_annotations);
    }

    struct NotFixableMask
    {
        bool visit(const UserMask &) const
        {
            return false;
        }

        bool visit(const UnacceptedMask &) const
        {
            return false;
        }

        bool visit(const RepositoryMask &) const
        {
            return false;
        }

        bool visit(const UnsupportedMask &) const
        {
            return true;
        }

        bool visit(const AssociationMask &) const
        {
            return true;
        }
    };

    struct MaskedByKeyVisitor
    {
        void visit(const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > & k)
        {
            cout << "            " << k.human_name() << " " << *k.value() << endl;
        }

        void visit(const MetadataValueKey<std::string> & k)
        {
            cout << "            " << k.human_name() << " " << k.value() << endl;
        }

        void visit(const MetadataValueKey<SlotName> & k)
        {
            cout << "            " << k.human_name() << " " << k.value() << endl;
        }

        void visit(const MetadataValueKey<long> & k)
        {
            cout << "            " << k.human_name() << " " << k.value() << endl;
        }

        void visit(const MetadataValueKey<bool> & k)
        {
            cout << "            " << k.human_name() << " " << k.value() << endl;
        }

        void visit(const MetadataSectionKey & k)
        {
            cout << "            " << k.human_name() << endl;
        }

        void visit(const MetadataTimeKey & k)
        {
            cout << "            " << k.human_name() << " " << pretty_print_time(k.value().seconds()) << endl;
        }

        void visit(const MetadataValueKey<std::tr1::shared_ptr<const Contents> > & k)
        {
            cout << "            " << k.human_name() << endl;
        }

        void visit(const MetadataValueKey<std::tr1::shared_ptr<const RepositoryMaskInfo> > & k)
        {
            cout << "            " << k.value()->mask_file() << endl;
            for (Sequence<std::string>::ConstIterator l(k.value()->comment()->begin()), l_end(k.value()->comment()->end()) ;
                    l != l_end ; ++l)
                cout << "            " << *l << endl;
        }

        void visit(const MetadataValueKey<FSEntry> & k)
        {
            cout << "            " << k.human_name() << " " << k.value() << endl;
        }

        void visit(const MetadataCollectionKey<Set<std::string> > & k)
        {
            ColourFormatter formatter(0);
            cout << "            " << k.human_name() << " " << k.pretty_print_flat(formatter) << endl;
        }

        void visit(const MetadataCollectionKey<Sequence<std::string> > & k)
        {
            ColourFormatter formatter(0);
            cout << "            " << k.human_name() << " " << k.pretty_print_flat(formatter) << endl;
        }

        void visit(const MetadataCollectionKey<FSEntrySequence> & k)
        {
            ColourFormatter formatter(0);
            cout << "            " << k.human_name() << " " << k.pretty_print_flat(formatter) << endl;
        }

        void visit(const MetadataCollectionKey<PackageIDSequence> & k)
        {
            ColourFormatter formatter(0);
            cout << "            " << k.human_name() << " " << k.pretty_print_flat(formatter) << endl;
        }

        void visit(const MetadataSpecTreeKey<FetchableURISpecTree> & k)
        {
            ColourFormatter formatter(0);
            cout << "            " << k.human_name() << " " << k.pretty_print_flat(formatter) << endl;
        }

        void visit(const MetadataSpecTreeKey<SimpleURISpecTree> & k)
        {
            ColourFormatter formatter(0);
            cout << "            " << k.human_name() << " " << k.pretty_print_flat(formatter) << endl;
        }

        void visit(const MetadataSpecTreeKey<LicenseSpecTree> & k)
        {
            ColourFormatter formatter(0);
            cout << "            " << k.human_name() << " " << k.pretty_print_flat(formatter) << endl;
        }

        void visit(const MetadataSpecTreeKey<DependencySpecTree> & k)
        {
            ColourFormatter formatter(0);
            cout << "            " << k.human_name() << " " << k.pretty_print_flat(formatter) << endl;
        }

        void visit(const MetadataCollectionKey<KeywordNameSet> & k)
        {
            ColourFormatter formatter(0);
            cout << "            " << k.human_name() << " " << k.pretty_print_flat(formatter) << endl;
        }

        void visit(const MetadataSpecTreeKey<ProvideSpecTree> & k)
        {
            ColourFormatter formatter(0);
            cout << "            " << k.human_name() << " " << k.pretty_print_flat(formatter) << endl;
        }

        void visit(const MetadataSpecTreeKey<PlainTextSpecTree> & k)
        {
            ColourFormatter formatter(0);
            cout << "            " << k.human_name() << " " << k.pretty_print_flat(formatter) << endl;
        }

        void visit(const MetadataValueKey<std::tr1::shared_ptr<const Choices> > & k)
        {
            ColourFormatter formatter(0);
            cout << "            " << k.human_name() << endl;
        }
    };

    struct MaskedByVisitor
    {
        void visit(const UserMask & m) const
        {
            cout << "        Masked by " << m.description() << endl;
        }

        void visit(const RepositoryMask & m) const
        {
            MaskedByKeyVisitor v;
            cout << "        Masked by " << m.description() << endl;
            if (m.mask_key())
                m.mask_key()->accept(v);
        }

        void visit(const UnacceptedMask & m) const
        {
            MaskedByKeyVisitor v;
            cout << "        Masked by " << m.description() << endl;
            if (m.unaccepted_key())
                m.unaccepted_key()->accept(v);
        }

        void visit(const UnsupportedMask & m) const
        {
            cout << "        Masked by " << m.description() << " (" << m.explanation() << ")" << endl;
        }

        void visit(const AssociationMask & m) const
        {
            cout << "        Masked by " << m.description() <<
                " (associated package '" << *m.associated_package() << "*)" << endl;
        }
    };

    void display_unable_to_make_decision(
            const std::tr1::shared_ptr<Environment> & env,
            const std::tr1::shared_ptr<const Resolution> & resolution,
            const UnableToMakeDecision & d,
            const bool untaken)
    {
        if (untaken)
            cout << "(!) " << c::bold_red() << resolution->resolvent() << c::normal() << endl;
        else
            cout << "!   " << c::bold_red() << resolution->resolvent() << c::normal() << endl;

        display_reasons(resolution, true);

        if (d.unsuitable_candidates()->empty())
            return;

        cout << "    Unsuitable candidates:" << endl;
        for (UnsuitableCandidates::ConstIterator u(d.unsuitable_candidates()->begin()),
                u_end(d.unsuitable_candidates()->end()) ;
                u != u_end ; ++u)
        {
            std::string colour;
            if (! u->unmet_constraints()->empty())
                colour = c::red();
            else if (u->package_id()->masked())
            {
                NotFixableMask is_not_fixable_mask;
                if (indirect_iterator(u->package_id()->end_masks()) == std::find_if(
                            indirect_iterator(u->package_id()->begin_masks()),
                            indirect_iterator(u->package_id()->end_masks()),
                            accept_visitor_returning<bool>(is_not_fixable_mask)))
                    colour = c::bold_red();
                else
                    colour = c::red();
            }

            cout << "      * " << colour << *u->package_id() << c::normal() << endl;
            for (PackageID::MasksConstIterator m(u->package_id()->begin_masks()),
                    m_end(u->package_id()->end_masks()) ;
                    m != m_end ; ++m)
                (*m)->accept(MaskedByVisitor());

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

    void display_choices_to_explain(
            const std::tr1::shared_ptr<Environment> &,
            const DisplayResolutionCommandLine &,
            const ChoicesToExplain & choices_to_explain)
    {
        Context context("When displaying choices to explain:");

        for (ChoicesToExplain::const_iterator p(choices_to_explain.begin()), p_end(choices_to_explain.end()) ;
                p != p_end ; ++p)
        {
            cout << p->first << ":" << endl;
            for (ChoiceValuesToExplain::const_iterator v(p->second.begin()), v_end(p->second.end()) ;
                    v != v_end ; ++v)
            {
                bool all_same(true);
                const std::tr1::shared_ptr<const ChoiceValue> first_choice_value(
                        (*v->second->begin())->choices_key()->value()->find_by_name_with_prefix(v->first));
                std::string description(first_choice_value->description());
                for (PackageIDSequence::ConstIterator w(next(v->second->begin())), w_end(v->second->end()) ;
                        w != w_end ; ++w)
                    if ((*w)->choices_key()->value()->find_by_name_with_prefix(v->first)->description() != description)
                    {
                        all_same = false;
                        break;
                    }

                if (all_same)
                    cout << "    " << std::left << std::setw(30) << (stringify(first_choice_value->unprefixed_name())
                            + ":") << " " << description << endl;
                else
                {
                    cout << "    " << first_choice_value->unprefixed_name() << ":" << endl;
                    for (PackageIDSequence::ConstIterator w(v->second->begin()), w_end(v->second->end()) ;
                            w != w_end ; ++w)
                    {
                        const std::tr1::shared_ptr<const ChoiceValue> value(
                                (*w)->choices_key()->value()->find_by_name_with_prefix(v->first));
                        cout << "        " << std::left << std::setw(30) <<
                            ((*w)->canonical_form(idcf_no_version) + ":") << " " << value->description() << endl;
                    }
                }
            }

            cout << endl;
        }
    }

    std::pair<std::tr1::shared_ptr<const ConfirmableDecision>, std::tr1::shared_ptr<const OrdererNotes> >
    get_decision_and_notes(const std::tr1::shared_ptr<const ConfirmableDecision> & d)
    {
        return std::make_pair(d, make_null_shared_ptr());
    }

    std::pair<std::tr1::shared_ptr<const ConfirmableDecision>, std::tr1::shared_ptr<const OrdererNotes> >
    get_decision_and_notes(const std::pair<std::tr1::shared_ptr<const ConfirmableDecision>, std::tr1::shared_ptr<const OrdererNotes> > & d)
    {
        return d;
    }

    void display_one_break(
            const std::tr1::shared_ptr<Environment> &,
            const DisplayResolutionCommandLine &,
            const std::tr1::shared_ptr<const Resolution> & resolution,
            const BreakDecision & decision,
            const bool more_annotations,
            const bool untaken)
    {
        if (untaken)
            cout << "(X) " << c::bold_red() << decision.resolvent().package() << c::normal() << " ";
        else
            cout << "X   " << c::bold_red() << decision.resolvent().package() << c::normal() << " ";

        cout << decision.existing_id()->canonical_form(idcf_no_name) << endl;
        cout << "    Will be broken by uninstalls:" << endl;
        display_reasons(resolution, more_annotations);
        display_confirmations(decision);
    }

    struct DisplayAVisitor
    {
        const std::tr1::shared_ptr<Environment> env;
        const DisplayResolutionCommandLine & cmdline;
        const std::tr1::shared_ptr<const Resolution> resolution;
        const bool more_annotations;
        const bool unconfirmed;
        const bool untaken;
        const std::string cycle_breaking;
        ChoicesToExplain & choices_to_explain;

        DisplayAVisitor(
                const std::tr1::shared_ptr<Environment> & e,
                const DisplayResolutionCommandLine & c,
                const std::tr1::shared_ptr<const Resolution> & r,
                bool m,
                bool uc,
                bool ut,
                const std::string & s,
                ChoicesToExplain & x) :
            env(e),
            cmdline(c),
            resolution(r),
            more_annotations(m),
            unconfirmed(uc),
            untaken(ut),
            cycle_breaking(s),
            choices_to_explain(x)
        {
        }

        void visit(const ChangesToMakeDecision & changes_to_make_decision)
        {
            display_one_installish(
                    env,
                    cmdline,
                    changes_to_make_decision,
                    resolution,
                    more_annotations,
                    unconfirmed,
                    untaken,
                    cycle_breaking,
                    choices_to_explain);
        }


        void visit(const RemoveDecision & remove_decision)
        {
            display_one_uninstall(
                    env,
                    cmdline,
                    resolution,
                    remove_decision,
                    more_annotations,
                    untaken);
        }

        void visit(const BreakDecision & break_decision)
        {
            display_one_break(
                    env,
                    cmdline,
                    resolution,
                    break_decision,
                    more_annotations,
                    untaken);
        }

        void visit(const NothingNoChangeDecision &) PALUDIS_ATTRIBUTE((noreturn))
        {
            throw InternalError(PALUDIS_HERE, "not allowed");
        }

        void visit(const ExistingNoChangeDecision &) PALUDIS_ATTRIBUTE((noreturn))
        {
            throw InternalError(PALUDIS_HERE, "not allowed");
        }

        void visit(const UnableToMakeDecision &) PALUDIS_ATTRIBUTE((noreturn))
        {
            throw InternalError(PALUDIS_HERE, "not allowed");
        }
    };

    template <typename Decisions_>
    void display_a_changes_and_removes(
            const std::tr1::shared_ptr<Environment> & env,
            const std::tr1::shared_ptr<const Resolved> & resolved,
            const std::tr1::shared_ptr<Decisions_> & decisions,
            const DisplayResolutionCommandLine & cmdline,
            ChoicesToExplain & choices_to_explain,
            const bool more_annotations,
            const bool unconfirmed,
            const bool untaken)
    {
        Context context("When displaying changes and removes:");

        if (untaken)
            cout << "I did not take the following:" << endl << endl;
        else if (unconfirmed)
            cout << "I cannot proceed without being permitted to do the following:" << endl << endl;
        else
            cout << "These are the actions I will take, in order:" << endl << endl;

        bool any(false);
        for (typename Decisions_::ConstIterator i(decisions->begin()), i_end(decisions->end()) ;
                i != i_end ; ++i)
        {
            any = true;

            const std::pair<
                std::tr1::shared_ptr<const ConfirmableDecision>,
                std::tr1::shared_ptr<const OrdererNotes> > star_i(get_decision_and_notes(*i));

            DisplayAVisitor v(
                    env,
                    cmdline,
                    *resolved->resolutions_by_resolvent()->find(star_i.first->resolvent()),
                    more_annotations,
                    unconfirmed,
                    untaken,
                    star_i.second ? star_i.second->cycle_breaking() : "",
                    choices_to_explain);
            star_i.first->accept(v);
        }

        if (! any)
            cout << "(nothing to do)" << endl;

        cout << endl;
    }

    void display_changes_and_removes(
            const std::tr1::shared_ptr<Environment> & env,
            const std::tr1::shared_ptr<const Resolved> & resolved,
            const DisplayResolutionCommandLine & cmdline,
            ChoicesToExplain & choices_to_explain)
    {
        display_a_changes_and_removes(env, resolved, resolved->taken_change_or_remove_decisions(),
                cmdline, choices_to_explain, false, false, false);
    }

    void display_untaken_changes_and_removes(
            const std::tr1::shared_ptr<Environment> & env,
            const std::tr1::shared_ptr<const Resolved> & resolved,
            const DisplayResolutionCommandLine & cmdline,
            ChoicesToExplain & choices_to_explain)
    {
        if (! resolved->untaken_change_or_remove_decisions()->empty())
            display_a_changes_and_removes(env, resolved, resolved->untaken_change_or_remove_decisions(),
                    cmdline, choices_to_explain, true, false, true);
    }

    void display_an_errors(
            const std::tr1::shared_ptr<Environment> & env,
            const std::tr1::shared_ptr<const Resolved> & resolved,
            const std::tr1::shared_ptr<const Decisions<UnableToMakeDecision> > & decisions,
            const DisplayResolutionCommandLine &,
            const bool untaken)
    {
        Context context("When displaying errors:");

        if (untaken)
            cout << "I encountered the following errors for untaken packages:" << endl << endl;
        else
            cout << "I encountered the following errors:" << endl << endl;

        bool any(false);
        for (Decisions<UnableToMakeDecision>::ConstIterator i(decisions->begin()),
                i_end(decisions->end()) ;
                i != i_end ; ++i)
        {
            any = true;

            display_unable_to_make_decision(
                    env,
                    *resolved->resolutions_by_resolvent()->find((*i)->resolvent()),
                    **i,
                    untaken);
        }

        cout << endl;
    }

    void display_taken_errors(
            const std::tr1::shared_ptr<Environment> & env,
            const std::tr1::shared_ptr<const Resolved> & resolved,
            const DisplayResolutionCommandLine & cmdline)
    {
        if (! resolved->taken_unable_to_make_decisions()->empty())
            display_an_errors(env, resolved, resolved->taken_unable_to_make_decisions(), cmdline, false);
    }

    void display_untaken_errors(
            const std::tr1::shared_ptr<Environment> & env,
            const std::tr1::shared_ptr<const Resolved> & resolved,
            const DisplayResolutionCommandLine & cmdline)
    {
        if (! resolved->untaken_unable_to_make_decisions()->empty())
            display_an_errors(env, resolved, resolved->untaken_unable_to_make_decisions(), cmdline, true);
    }

    void display_taken_changes_requiring_confirmation(
            const std::tr1::shared_ptr<Environment> & env,
            const std::tr1::shared_ptr<const Resolved> & resolved,
            const DisplayResolutionCommandLine & cmdline)
    {
        ChoicesToExplain ignore_choices_to_explain;
        if (! resolved->taken_unconfirmed_decisions()->empty())
            display_a_changes_and_removes(env, resolved, resolved->taken_unconfirmed_decisions(),
                    cmdline, ignore_choices_to_explain, true, true, false);
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
        const std::tr1::shared_ptr<const Resolved> & maybe_resolved
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

    std::tr1::shared_ptr<const Resolved> resolved(maybe_resolved);
    if (! resolved)
    {
        if (getenv_with_default("PALUDIS_SERIALISED_RESOLUTION_FD", "").empty())
            throw args::DoHelp("PALUDIS_SERIALISED_RESOLUTION_FD must be provided");

        int fd(destringify<int>(getenv_with_default("PALUDIS_SERIALISED_RESOLUTION_FD", "")));
        SafeIFStream deser_stream(fd);
        Deserialiser deserialiser(env.get(), deser_stream);
        Deserialisation deserialisation("Resolved", deserialiser);
        resolved = make_shared_copy(Resolved::deserialise(deserialisation));
        close(fd);
    }

    ChoicesToExplain choices_to_explain;
    display_changes_and_removes(env, resolved, cmdline, choices_to_explain);
    display_untaken_changes_and_removes(env, resolved, cmdline, choices_to_explain);
    display_choices_to_explain(env, cmdline, choices_to_explain);
    display_taken_errors(env, resolved, cmdline);
    display_untaken_errors(env, resolved, cmdline);
    display_taken_changes_requiring_confirmation(env, resolved, cmdline);
    display_explanations(env, resolved, cmdline);

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

