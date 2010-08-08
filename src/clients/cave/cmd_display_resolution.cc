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
#include <paludis/args/do_help.hh>
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
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/util/return_literal_function.hh>
#include <paludis/util/enum_iterator.hh>
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
#include <paludis/resolver/change_by_resolvent.hh>
#include <paludis/resolver/match_qpns.hh>
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
#include <paludis/action.hh>
#include <paludis/output_manager_from_environment.hh>
#include <paludis/output_manager.hh>
#include <paludis/changed_choices.hh>
#include <paludis/mask_utils.hh>

#include <set>
#include <iterator>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <map>
#include <limits>

using namespace paludis;
using namespace cave;
using namespace paludis::resolver;

using std::cout;
using std::endl;

typedef std::map<ChoiceNameWithPrefix, std::shared_ptr<PackageIDSequence> > ChoiceValuesToExplain;
typedef std::map<std::string, ChoiceValuesToExplain> ChoicesToExplain;
typedef std::map<std::string, int> AlreadyCycleNotes;

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
            const std::shared_ptr<const MetadataSectionKey> & section,
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

    std::string stringify_change_by_resolvent(const ChangeByResolvent & r)
    {
        return stringify(*r.package_id());
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

        std::pair<std::string, Tribool> annotate(
                const std::shared_ptr<const MetadataSectionKey> & key,
                const std::pair<std::string, Tribool> unannotated) const
        {
            if ((! key) || (! more_annotations))
                return unannotated;

            std::pair<std::string, Tribool> result(unannotated);

            std::string description_annotation(get_annotation(key, "description"));
            if (! description_annotation.empty())
            {
                result.first = result.first + ": \"" + description_annotation + "\"";
                result.second = true;
            }

            return result;
        }

        std::pair<std::string, Tribool> visit(const DependencyReason & r) const
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

        std::pair<std::string, Tribool> visit(const DependentReason & r) const
        {
            return std::make_pair("dependent upon " + stringify(*r.id_and_resolvent_being_removed().package_id()), true);
        }

        std::pair<std::string, Tribool> visit(const WasUsedByReason & r) const
        {
            if (r.ids_and_resolvents_being_removed()->empty())
                return std::make_pair("was unused", true);
            else
                return std::make_pair("was used by " + join(r.ids_and_resolvents_being_removed()->begin(),
                            r.ids_and_resolvents_being_removed()->end(),
                            ", ", stringify_change_by_resolvent), true);
        }

        std::pair<std::string, Tribool> visit(const TargetReason & r) const
        {
            return std::make_pair("target" + (r.extra_information().empty() ? "" : " (" + r.extra_information() + ")"), true);
        }

        std::pair<std::string, Tribool> visit(const SetReason & r) const
        {
            std::pair<std::string, Tribool> rr(r.reason_for_set()->accept_returning<std::pair<std::string, Tribool> >(*this));
            return std::make_pair(rr.first + " (" + stringify(r.set_name()) + ")", rr.second);
        }

        std::pair<std::string, Tribool> visit(const LikeOtherDestinationTypeReason & r) const
        {
            std::pair<std::string, Tribool> rr(r.reason_for_other()->accept_returning<std::pair<std::string, Tribool> >(*this));
            return std::make_pair(rr.first + " (to be like " + stringify(r.other_resolvent()) + ")", rr.second);
        }

        std::pair<std::string, Tribool> visit(const ViaBinaryReason & r) const
        {
            return std::make_pair("to install via binary for " + stringify(r.other_resolvent()), indeterminate);
        }

        std::pair<std::string, Tribool> visit(const PresetReason & r) const
        {
            std::pair<std::string, Tribool> rr("", indeterminate);
            if (r.maybe_reason_for_preset())
                rr = r.maybe_reason_for_preset()->accept_returning<std::pair<std::string, Tribool> >(*this);

            rr.first = r.maybe_explanation() + (r.maybe_explanation().empty() || rr.first.empty() ? "" : " ")
                + rr.first;

            return rr;
        }
    };

    struct IDForDecisionOrNullVisitor
    {
        const std::shared_ptr<const PackageID> visit(const ExistingNoChangeDecision & d) const
        {
            return d.existing_id();
        }

        const std::shared_ptr<const PackageID> visit(const BreakDecision & d) const
        {
            return d.existing_id();
        }

        const std::shared_ptr<const PackageID> visit(const ChangesToMakeDecision & d) const
        {
            return d.origin_id();
        }

        const std::shared_ptr<const PackageID> visit(const UnableToMakeDecision &) const
        {
            return make_null_shared_ptr();
        }

        const std::shared_ptr<const PackageID> visit(const RemoveDecision &) const
        {
            return make_null_shared_ptr();
        }

        const std::shared_ptr<const PackageID> visit(const NothingNoChangeDecision &) const
        {
            return make_null_shared_ptr();
        }
    };

    const std::shared_ptr<const PackageID> id_for_decision_or_null(const Decision & d)
    {
        return d.accept_returning<std::shared_ptr<const PackageID> >(IDForDecisionOrNullVisitor());
    }

    bool decision_matches_spec(
            const std::shared_ptr<Environment> & env,
            const Resolvent & resolvent,
            const Decision & decision,
            const PackageDepSpec & spec)
    {
        const std::shared_ptr<const PackageID> maybe_id(id_for_decision_or_null(decision));
        if (maybe_id)
            return match_package(*env, spec, *maybe_id, { });
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

            case dt_install_to_chroot:
                result << ", installing to chroot";
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
            cout << (*c)->reason()->accept_returning<std::pair<std::string, Tribool> >(g).first;
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
            cout << "        Use origin ID " << *d.origin_id();
            if (d.if_via_new_binary_in())
                cout << " via binary created in " << *d.if_via_new_binary_in();
            cout << endl;
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
            const std::shared_ptr<Environment> & env,
            const std::shared_ptr<const Resolved> & resolved,
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
            PackageDepSpec spec(parse_user_package_dep_spec(*i, env.get(), { updso_allow_wildcards }));
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
            const std::shared_ptr<Environment> &,
            const DisplayResolutionCommandLine & cmdline,
            const std::shared_ptr<const PackageID> & id,
            const bool is_new)
    {
        if (id->short_description_key() && ! id->short_description_key()->value().empty())
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
            const std::shared_ptr<Environment> &,
            const DisplayResolutionCommandLine & cmdline,
            const std::shared_ptr<const PackageID> & id,
            const std::shared_ptr<const ChangedChoices> & changed_choices,
            const std::shared_ptr<const PackageID> & old_id,
            ChoicesToExplain & choices_to_explain
            )
    {
        if (! id->choices_key())
            return;

        ColourFormatter formatter(0);

        std::shared_ptr<const Choices> old_choices;
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

                Tribool changed_state(indeterminate);
                if (changed_choices)
                    changed_state = changed_choices->overridden_value((*i)->name_with_prefix());

                std::string t;
                if ((changed_state.is_indeterminate() && (*i)->enabled()) || (changed_state.is_true()))
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
                        std::shared_ptr<const ChoiceValue> old_choice(
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

                if (! changed_state.is_indeterminate())
                    t = t + c::bold_red() + " (!)" + c::normal();

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
                                (*i)->name_with_prefix(), std::make_shared<PackageIDSequence>())).first->second->push_back(id);
            }
        }

        if (s.empty())
            return;

        if (changed_choices)
            cout << "    " << c::bold_red() << "Changes needed: " << c::normal() << s << endl;
        else
            cout << "    " << s << endl;
    }

    void display_reasons(
            const std::shared_ptr<const Resolution> & resolution,
            const bool more_annotations
            )
    {
        std::set<std::string> reasons, special_reasons;
        for (Constraints::ConstIterator c(resolution->constraints()->begin()),
                c_end(resolution->constraints()->end()) ;
                c != c_end ; ++c)
        {
            ReasonNameGetter g(false, more_annotations);
            std::pair<std::string, Tribool> r((*c)->reason()->accept_returning<std::pair<std::string, Tribool> >(g));
            if (r.first.empty())
                continue;

            if (r.second.is_true())
                special_reasons.insert(r.first);
            else if (r.second.is_false())
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

        std::string visit(const RemoveSystemPackageConfirmation &) const
        {
            return "--uninstalls-may-break system";
        }

        std::string visit(const MaskedConfirmation &) const
        {
            return "being unmasked";
        }

        std::string visit(const ChangedChoicesConfirmation &) const
        {
            return "being reconfigured";
        }
    };

    std::string stringify_confirmation(const RequiredConfirmation & c)
    {
        return c.accept_returning<std::string>(DisplayConfirmationVisitor());
    }

    void display_confirmations(
            const ConfirmableDecision & decision)
    {
        const std::shared_ptr<const RequiredConfirmations> r(decision.required_confirmations_if_any());
        if (r && ! r->empty())
            cout << c::bold_red() << "    Cannot proceed without: " << c::normal() <<
                join(indirect_iterator(r->begin()), indirect_iterator(r->end()), ", ", stringify_confirmation) << endl;
    }

    void display_untaken_change(
            const ChangesToMakeDecision &)
    {
        cout << c::bold_green() << "    Take using: " << c::normal() << "--take" << endl;
    }

    struct IsPurgeVisitor
    {
        bool visit(const TargetReason &) const
        {
            return false;
        }

        bool visit(const SetReason & r) const
        {
            return r.reason_for_set()->accept_returning<bool>(*this);
        }

        bool visit(const LikeOtherDestinationTypeReason & r) const
        {
            return r.reason_for_other()->accept_returning<bool>(*this);
        }

        bool visit(const PresetReason &) const
        {
            return false;
        }

        bool visit(const DependencyReason &) const
        {
            return false;
        }

        bool visit(const ViaBinaryReason &) const
        {
            return false;
        }

        bool visit(const DependentReason &) const
        {
            return false;
        }

        bool visit(const WasUsedByReason &) const
        {
            return true;
        }
    };

    void display_untaken_remove(
            const RemoveDecision &)
    {
        cout << c::bold_green() << "    Take using: " << c::normal() << "--purge" << endl;
    }

    struct MaskedByKeyVisitor
    {
        const std::string indent;

        void visit(const MetadataValueKey<std::shared_ptr<const PackageID> > & k)
        {
            cout << indent << k.human_name() << " " << *k.value() << endl;
        }

        void visit(const MetadataValueKey<std::string> & k)
        {
            cout << indent << k.human_name() << " " << k.value() << endl;
        }

        void visit(const MetadataValueKey<SlotName> & k)
        {
            cout << indent << k.human_name() << " " << k.value() << endl;
        }

        void visit(const MetadataValueKey<long> & k)
        {
            cout << indent << k.human_name() << " " << k.value() << endl;
        }

        void visit(const MetadataValueKey<bool> & k)
        {
            cout << indent << k.human_name() << " " << k.value() << endl;
        }

        void visit(const MetadataSectionKey & k)
        {
            cout << indent << k.human_name() << endl;
        }

        void visit(const MetadataTimeKey & k)
        {
            cout << indent << k.human_name() << " " << pretty_print_time(k.value().seconds()) << endl;
        }

        void visit(const MetadataValueKey<std::shared_ptr<const Contents> > & k)
        {
            cout << indent << k.human_name() << endl;
        }

        void visit(const MetadataValueKey<std::shared_ptr<const RepositoryMaskInfo> > & k)
        {
            cout << indent << k.value()->mask_file() << endl;
            for (Sequence<std::string>::ConstIterator l(k.value()->comment()->begin()), l_end(k.value()->comment()->end()) ;
                    l != l_end ; ++l)
                cout << indent << *l << endl;
        }

        void visit(const MetadataValueKey<FSEntry> & k)
        {
            cout << indent << k.human_name() << " " << k.value() << endl;
        }

        void visit(const MetadataCollectionKey<Set<std::string> > & k)
        {
            ColourFormatter formatter(0);
            cout << indent << k.human_name() << " " << k.pretty_print_flat(formatter) << endl;
        }

        void visit(const MetadataCollectionKey<Sequence<std::string> > & k)
        {
            ColourFormatter formatter(0);
            cout << indent << k.human_name() << " " << k.pretty_print_flat(formatter) << endl;
        }

        void visit(const MetadataCollectionKey<FSEntrySequence> & k)
        {
            ColourFormatter formatter(0);
            cout << indent << k.human_name() << " " << k.pretty_print_flat(formatter) << endl;
        }

        void visit(const MetadataCollectionKey<PackageIDSequence> & k)
        {
            ColourFormatter formatter(0);
            cout << indent << k.human_name() << " " << k.pretty_print_flat(formatter) << endl;
        }

        void visit(const MetadataSpecTreeKey<FetchableURISpecTree> & k)
        {
            ColourFormatter formatter(0);
            cout << indent << k.human_name() << " " << k.pretty_print_flat(formatter) << endl;
        }

        void visit(const MetadataSpecTreeKey<SimpleURISpecTree> & k)
        {
            ColourFormatter formatter(0);
            cout << indent << k.human_name() << " " << k.pretty_print_flat(formatter) << endl;
        }

        void visit(const MetadataSpecTreeKey<LicenseSpecTree> & k)
        {
            ColourFormatter formatter(0);
            cout << indent << k.human_name() << " " << k.pretty_print_flat(formatter) << endl;
        }

        void visit(const MetadataSpecTreeKey<DependencySpecTree> & k)
        {
            ColourFormatter formatter(0);
            cout << indent << k.human_name() << " " << k.pretty_print_flat(formatter) << endl;
        }

        void visit(const MetadataCollectionKey<KeywordNameSet> & k)
        {
            ColourFormatter formatter(0);
            cout << indent << k.human_name() << " " << k.pretty_print_flat(formatter) << endl;
        }

        void visit(const MetadataSpecTreeKey<ProvideSpecTree> & k)
        {
            ColourFormatter formatter(0);
            cout << indent << k.human_name() << " " << k.pretty_print_flat(formatter) << endl;
        }

        void visit(const MetadataSpecTreeKey<PlainTextSpecTree> & k)
        {
            ColourFormatter formatter(0);
            cout << indent << k.human_name() << " " << k.pretty_print_flat(formatter) << endl;
        }

        void visit(const MetadataValueKey<std::shared_ptr<const Choices> > & k)
        {
            ColourFormatter formatter(0);
            cout << indent << k.human_name() << endl;
        }
    };

    struct MaskedByVisitor
    {
        const std::string colour;
        const std::string indent;

        void visit(const UserMask & m) const
        {
            cout << colour << indent << "Masked by " << c::normal() << m.description() << endl;
        }

        void visit(const RepositoryMask & m) const
        {
            MaskedByKeyVisitor v{indent + "    "};
            cout << colour << indent << "Masked by " << c::normal() << m.description() << endl;
            if (m.mask_key())
                m.mask_key()->accept(v);
        }

        void visit(const UnacceptedMask & m) const
        {
            MaskedByKeyVisitor v{indent + "    "};
            cout << colour << indent << "Masked by " << c::normal() << m.description() << endl;
            if (m.unaccepted_key())
                m.unaccepted_key()->accept(v);
        }

        void visit(const UnsupportedMask & m) const
        {
            cout << colour << indent << "Masked by " << c::normal() << m.description() << " (" << m.explanation() << ")" << endl;
        }

        void visit(const AssociationMask & m) const
        {
            cout << colour << indent << "Masked by " << c::normal() << m.description() <<
                " (associated package '" << *m.associated_package() << "')" << endl;
        }
    };

    void display_masks(
            const std::shared_ptr<Environment> &,
            const ChangesToMakeDecision & decision)
    {
        if (! decision.origin_id()->masked())
            return;

        for (auto m(decision.origin_id()->begin_masks()), m_end(decision.origin_id()->end_masks()) ;
                m != m_end ; ++m)
            (*m)->accept(MaskedByVisitor{c::bold_red(), "    "});
    }

    struct Totals
    {
        std::set<FSEntry> download_files;
        bool download_overflow;
        unsigned long download_size;

        std::map<ChangeType, int> installs_ct_count;
        int binary_installs_count, uninstalls_count;

        Totals() :
            download_overflow(false),
            download_size(0),
            binary_installs_count(0),
            uninstalls_count(0)
        {
        }
    };

    struct FindDistfilesSize :
        PretendFetchAction
    {
        std::shared_ptr<Totals> totals;
        unsigned long size;
        bool overflow;

        FindDistfilesSize(const FetchActionOptions & o, const std::shared_ptr<Totals> & a) :
            PretendFetchAction(o),
            totals(a),
            size(0),
            overflow(false)
        {
        }

        void will_fetch(const FSEntry & destination, const unsigned long size_in_bytes)
        {
            if (totals->download_files.end() != totals->download_files.find(destination))
                return;
            totals->download_files.insert(destination);
            unsigned long new_size(size + size_in_bytes);
            if (new_size < size)
                overflow = true;
            else
                size = new_size;
        }
    };

    void display_downloads(
            const std::shared_ptr<Environment> & env,
            const DisplayResolutionCommandLine &,
            const std::shared_ptr<const PackageID> & id,
            const std::shared_ptr<Totals> & totals
            )
    {
        if (! id->supports_action(SupportsActionTest<PretendFetchAction>()))
            return;

        OutputManagerFromEnvironment output_manager_holder(env.get(), id, oe_exclusive, { });

        FindDistfilesSize action(make_named_values<FetchActionOptions>(
                    n::errors() = std::make_shared<Sequence<FetchActionFailure>>(),
                    n::exclude_unmirrorable() = false,
                    n::fetch_parts() = FetchParts() + fp_regulars,
                    n::ignore_not_in_manifest() = false,
                    n::ignore_unfetched() = false,
                    n::make_output_manager() = std::ref(output_manager_holder),
                    n::safe_resume() = true,
                    n::want_phase() = std::bind(return_literal_function(wp_yes))
                    ),
                totals);
        id->perform_action(action);

        if (output_manager_holder.output_manager_if_constructed())
            output_manager_holder.output_manager_if_constructed()->succeeded();

        if (0 == action.size)
            return;

        if (action.overflow)
            cout << "    More than " << pretty_print_bytes(std::numeric_limits<unsigned long>::max()) << " to download" << endl;
        else
            cout << "    " << pretty_print_bytes(action.size) << " to download" << endl;

        if (action.overflow)
            totals->download_overflow = true;
        else
        {
            unsigned long new_size(totals->download_size + action.size);
            if (new_size < totals->download_size)
                totals->download_overflow = true;
            else
                totals->download_size = new_size;
        }
    }

    void display_one_installish(
            const std::shared_ptr<Environment> & env,
            const DisplayResolutionCommandLine & cmdline,
            const ChangesToMakeDecision & decision,
            const std::shared_ptr<const Resolution> & resolution,
            const bool more_annotations,
            const bool confirmations,
            const bool untaken,
            const bool unorderable,
            const std::string & cycle_notes,
            int cycle_notes_heading,
            ChoicesToExplain & choices_to_explain,
            const std::shared_ptr<Totals> & maybe_totals)
    {
        std::string x("X"), c;
        if (! decision.best())
            x = "-" + x;

        if (decision.origin_id()->masked() || decision.if_changed_choices())
            c = c::red();
        else
            do
            {
                switch (resolution->resolvent().destination_type())
                {
                    case dt_install_to_slash:
                        if (decision.if_via_new_binary_in())
                            c = c::yellow();
                        if (maybe_totals)
                            ++maybe_totals->installs_ct_count.insert(std::make_pair(decision.change_type(), 0)).first->second;
                        continue;

                    case dt_install_to_chroot:
                        c = c::blue();
                        if (maybe_totals)
                            ++maybe_totals->installs_ct_count.insert(std::make_pair(decision.change_type(), 0)).first->second;
                        continue;

                    case dt_create_binary:
                        c = c::green();
                        if (maybe_totals)
                            ++maybe_totals->binary_installs_count;
                        continue;

                    case last_dt:
                        break;
                }

                throw InternalError(PALUDIS_HERE, "bad destination_type. huh?");
            }
            while (false);

        if (untaken)
            x = "(" + x + ")";

        if (x.length() < 4)
            x = x + std::string(4 - x.length(), ' ');

        do
        {
            switch (decision.change_type())
            {
                case ct_new:
                    cout << c << x.replace(x.find('X'), 1, "n") << c::bold_blue();
                    continue;
                case ct_slot_new:
                    cout << c << x.replace(x.find('X'), 1, "s") << c::bold_blue();
                    continue;
                case ct_upgrade:
                    cout << c << x.replace(x.find('X'), 1, "u") << c::blue();
                    continue;
                case ct_reinstall:
                    cout << c << x.replace(x.find('X'), 1, "r") << c::yellow();
                    continue;
                case ct_downgrade:
                    cout << c << x.replace(x.find('X'), 1, "d") << c::bold_yellow();
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

        if ((! decision.destination()->replacing()->empty()) &&
                (*decision.destination()->replacing()->begin())->from_repositories_key() &&
                ! (*decision.destination()->replacing()->begin())->from_repositories_key()->value()->empty() &&
                (*decision.destination()->replacing()->begin())->from_repositories_key()->value()->end() ==
                (*decision.destination()->replacing()->begin())->from_repositories_key()->value()->find(stringify(
                        decision.origin_id()->repository()->name())))
        {
            cout << c::bold_yellow() << " (formerly from ::" << join(
                        (*decision.destination()->replacing()->begin())->from_repositories_key()->value()->begin(),
                        (*decision.destination()->replacing()->begin())->from_repositories_key()->value()->end(),
                        ", ::") << ")";
        }

        cout << c::normal() << " " << decision.origin_id()->canonical_form(idcf_version) <<
            " to " << decision.destination()->repository();

        if (decision.if_via_new_binary_in())
            cout << c::normal() << " via binary created in " << c::bold_normal()
                << *decision.if_via_new_binary_in() << c::normal();

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

        if (-1 != cycle_notes_heading)
            cout << " " << (unorderable ? c::bold_red() : c::bold_normal()) << "[cycle " << cycle_notes_heading << "]" << c::normal();

        cout << endl;

        std::shared_ptr<const PackageID> old_id;
        if (! decision.destination()->replacing()->empty())
            old_id = *decision.destination()->replacing()->begin();

        display_one_description(env, cmdline, decision.origin_id(), ! old_id);
        display_choices(env, cmdline, decision.origin_id(), decision.if_changed_choices(), old_id, choices_to_explain);
        display_reasons(resolution, more_annotations);
        display_masks(env, decision);
        if (maybe_totals)
            display_downloads(env, cmdline, decision.origin_id(), maybe_totals);
        if (untaken)
            display_untaken_change(decision);
        if (confirmations)
            display_confirmations(decision);
        if (! cycle_notes.empty())
            cout << "    " << (unorderable ? c::bold_red() : c::bold_normal()) << cycle_notes << c::normal() << endl;
    }

    void display_one_uninstall(
            const std::shared_ptr<Environment> &,
            const DisplayResolutionCommandLine &,
            const std::shared_ptr<const Resolution> & resolution,
            const RemoveDecision & decision,
            const bool more_annotations,
            const bool confirmations,
            const bool untaken,
            const bool unorderable,
            const std::string & cycle_notes,
            int cycle_notes_heading,
            const std::shared_ptr<Totals> & maybe_totals)
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

        if (-1 != cycle_notes_heading)
            cout << " " << (unorderable ? c::bold_red() : c::bold_normal()) << "[cycle " << cycle_notes_heading << "]" << c::normal();

        cout << endl;
        display_reasons(resolution, more_annotations);
        if (untaken)
            display_untaken_remove(decision);
        if (confirmations)
            display_confirmations(decision);
        if (! cycle_notes.empty())
            cout << "    " << c::bold_normal() << cycle_notes << c::normal() << endl;

        if (maybe_totals)
            ++maybe_totals->uninstalls_count;
    }

    void display_unable_to_make_decision(
            const std::shared_ptr<Environment> & env,
            const std::shared_ptr<const Resolution> & resolution,
            const UnableToMakeDecision & d,
            const bool untaken)
    {
        if (untaken)
            cout << "(!) " << c::red() << resolution->resolvent() << c::normal() << endl;
        else
            cout << "!   " << c::bold_red() << resolution->resolvent() << c::normal() << endl;

        display_reasons(resolution, true);

        if (untaken || d.unsuitable_candidates()->empty())
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
                if (not_strongly_masked(u->package_id()))
                    colour = c::bold_red();
                else
                    colour = c::red();
            }

            cout << "      * " << colour << *u->package_id() << c::normal() << endl;
            for (PackageID::MasksConstIterator m(u->package_id()->begin_masks()),
                    m_end(u->package_id()->end_masks()) ;
                    m != m_end ; ++m)
                (*m)->accept(MaskedByVisitor{"", "        "});

            std::set<std::string> duplicates;
            for (Constraints::ConstIterator c(u->unmet_constraints()->begin()),
                    c_end(u->unmet_constraints()->end()) ;
                    c != c_end ; ++c)
            {
                ReasonNameGetter g(false, true);
                std::string s(constraint_as_string(**c) + " from " +
                        (*c)->reason()->accept_returning<std::pair<std::string, Tribool> >(g).first);

                if (! duplicates.insert(s).second)
                    continue;

                cout << "        Did not meet " << s << endl;

                if ((*c)->spec().if_package() && (*c)->spec().if_package()->additional_requirements_ptr() &&
                        (! match_package(*env, *(*c)->spec().if_package(), *u->package_id(), { })) &&
                        match_package(*env, *(*c)->spec().if_package(), *u->package_id(), { mpo_ignore_additional_requirements }))
                {
                    for (AdditionalPackageDepSpecRequirements::ConstIterator a((*c)->spec().if_package()->additional_requirements_ptr()->begin()),
                            a_end((*c)->spec().if_package()->additional_requirements_ptr()->end()) ;
                            a != a_end ; ++a)
                    {
                        const std::pair<bool, std::string> p((*a)->requirement_met(env.get(), 0, *u->package_id(), 0));
                        if (p.first)
                            continue;

                        cout << "            " << p.second << endl;
                    }
                }
            }
        }
    }

    void display_choices_to_explain(
            const std::shared_ptr<Environment> &,
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
                const std::shared_ptr<const ChoiceValue> first_choice_value(
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
                        const std::shared_ptr<const ChoiceValue> value(
                                (*w)->choices_key()->value()->find_by_name_with_prefix(v->first));
                        cout << "        " << std::left << std::setw(30) <<
                            ((*w)->canonical_form(idcf_no_version) + ":") << " " << value->description() << endl;
                    }
                }
            }

            cout << endl;
        }
    }

    std::pair<std::shared_ptr<const ConfirmableDecision>, std::shared_ptr<const OrdererNotes> >
    get_decision_and_notes(const std::shared_ptr<const ConfirmableDecision> & d)
    {
        return std::make_pair(d, make_null_shared_ptr());
    }

    std::pair<std::shared_ptr<const ConfirmableDecision>, std::shared_ptr<const OrdererNotes> >
    get_decision_and_notes(const std::pair<std::shared_ptr<const ConfirmableDecision>, std::shared_ptr<const OrdererNotes> > & d)
    {
        return d;
    }

    void display_one_break(
            const std::shared_ptr<Environment> &,
            const DisplayResolutionCommandLine &,
            const std::shared_ptr<const Resolution> & resolution,
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
        const std::shared_ptr<Environment> env;
        const DisplayResolutionCommandLine & cmdline;
        const std::shared_ptr<const Resolution> resolution;
        const bool more_annotations;
        const bool unconfirmed;
        const bool untaken;
        const bool unorderable;
        const std::string cycle_notes;
        const int cycle_notes_heading;
        ChoicesToExplain & choices_to_explain;
        const std::shared_ptr<Totals> maybe_totals;

        DisplayAVisitor(
                const std::shared_ptr<Environment> & e,
                const DisplayResolutionCommandLine & c,
                const std::shared_ptr<const Resolution> & r,
                bool m,
                bool uc,
                bool ut,
                bool un,
                const std::string & cn,
                int ch,
                ChoicesToExplain & x,
                const std::shared_ptr<Totals> & d) :
            env(e),
            cmdline(c),
            resolution(r),
            more_annotations(m),
            unconfirmed(uc),
            untaken(ut),
            unorderable(un),
            cycle_notes(cn),
            cycle_notes_heading(ch),
            choices_to_explain(x),
            maybe_totals(d)
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
                    unorderable,
                    cycle_notes,
                    cycle_notes_heading,
                    choices_to_explain,
                    maybe_totals);
        }


        void visit(const RemoveDecision & remove_decision)
        {
            display_one_uninstall(
                    env,
                    cmdline,
                    resolution,
                    remove_decision,
                    more_annotations,
                    unconfirmed,
                    untaken,
                    unorderable,
                    cycle_notes,
                    cycle_notes_heading,
                    maybe_totals);
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
    };

    template <typename Decisions_>
    void display_a_changes_and_removes(
            const std::shared_ptr<Environment> & env,
            const std::shared_ptr<const Resolved> & resolved,
            const std::shared_ptr<Decisions_> & decisions,
            const DisplayResolutionCommandLine & cmdline,
            ChoicesToExplain & choices_to_explain,
            const bool more_annotations,
            const bool unconfirmed,
            const bool untaken,
            const bool unorderable,
            const std::shared_ptr<Totals> & maybe_totals,
            AlreadyCycleNotes & already_cycle_notes)
    {
        Context context("When displaying changes and removes:");

        if (untaken)
            cout << "I did not take the following:" << endl << endl;
        else if (unconfirmed)
            cout << "I cannot proceed without being permitted to do the following:" << endl << endl;
        else if (unorderable)
            cout << "I cannot provide a legal ordering for the following:" << endl << endl;
        else
            cout << "These are the actions I will take, in order:" << endl << endl;

        bool any(false);
        for (typename Decisions_::ConstIterator i(decisions->begin()), i_end(decisions->end()) ;
                i != i_end ; ++i)
        {
            any = true;

            const std::pair<
                std::shared_ptr<const ConfirmableDecision>,
                std::shared_ptr<const OrdererNotes> > star_i(get_decision_and_notes(*i));

            std::string cycle_notes;
            int cycle_notes_heading(-1);
            if (star_i.second && ! star_i.second->cycle_breaking().empty())
            {
                auto existing(already_cycle_notes.find(star_i.second->cycle_breaking()));
                if (existing != already_cycle_notes.end())
                    cycle_notes_heading = existing->second;
                else
                    std::tie(cycle_notes, cycle_notes_heading) = *already_cycle_notes.insert(std::make_pair(
                                star_i.second->cycle_breaking(), already_cycle_notes.size() + 1)).first;
            }

            DisplayAVisitor v(
                    env,
                    cmdline,
                    *resolved->resolutions_by_resolvent()->find(star_i.first->resolvent()),
                    more_annotations,
                    unconfirmed,
                    untaken,
                    unorderable,
                    cycle_notes,
                    cycle_notes_heading,
                    choices_to_explain,
                    maybe_totals);
            star_i.first->accept(v);
        }

        if (! any)
            cout << "(nothing to do)" << endl;

        cout << endl;
    }

    void display_changes_and_removes(
            const std::shared_ptr<Environment> & env,
            const std::shared_ptr<const Resolved> & resolved,
            const DisplayResolutionCommandLine & cmdline,
            ChoicesToExplain & choices_to_explain,
            const std::shared_ptr<Totals> & totals,
            AlreadyCycleNotes & already_cycle_notes)
    {
        display_a_changes_and_removes(env, resolved, resolved->taken_change_or_remove_decisions(),
                cmdline, choices_to_explain, false, false, false, false, totals, already_cycle_notes);
    }

    void display_totals(
            const std::shared_ptr<Totals> & totals)
    {
        if (totals->installs_ct_count.empty() && 0 == totals->binary_installs_count + totals->uninstalls_count)
            return;

        cout << "Total " ;
        bool need_comma(false);

        for (EnumIterator<ChangeType> e, e_end(last_ct) ;
                e != e_end ; ++e)
        {
            auto c(totals->installs_ct_count.find(*e));
            if (c == totals->installs_ct_count.end())
                continue;

            if (need_comma)
                cout << ", ";
            need_comma = true;
            cout << c->second;
            switch (c->first)
            {
                case ct_new:
                    cout << " new installs";
                    break;

                case ct_slot_new:
                    cout << " new slot installs";
                    break;

                case ct_upgrade:
                    cout << " upgrades";
                    break;

                case ct_reinstall:
                    cout << " reinstalls";
                    break;

                case ct_downgrade:
                    cout << " downgrades";
                    break;

                case last_ct:
                    throw InternalError(PALUDIS_HERE, "bad change_type. huh?");
            }
        }

        if (0 != totals->binary_installs_count)
        {
            if (need_comma)
                cout << ", ";
            need_comma = true;
            cout << totals->binary_installs_count << " binaries";
        }

        if (0 != totals->uninstalls_count)
        {
            if (need_comma)
                cout << ", ";
            need_comma = true;
            cout << totals->uninstalls_count << " uninstalls";
        }

        if (totals->download_overflow)
            cout << ", more than " << pretty_print_bytes(std::numeric_limits<unsigned long>::max()) << " to download";
        else if (0 != totals->download_size)
            cout << ", " << pretty_print_bytes(totals->download_size) << " to download";
        cout << endl << endl;
    }

    void display_unorderable_changes_and_removed(
            const std::shared_ptr<Environment> & env,
            const std::shared_ptr<const Resolved> & resolved,
            const DisplayResolutionCommandLine & cmdline,
            ChoicesToExplain & choices_to_explain,
            AlreadyCycleNotes & already_cycle_notes)
    {
        if (! resolved->taken_unorderable_decisions()->empty())
            display_a_changes_and_removes(env, resolved, resolved->taken_unorderable_decisions(),
                    cmdline, choices_to_explain, false, false, false, true, make_null_shared_ptr(), already_cycle_notes);
    }

    void display_untaken_changes_and_removes(
            const std::shared_ptr<Environment> & env,
            const std::shared_ptr<const Resolved> & resolved,
            const DisplayResolutionCommandLine & cmdline,
            ChoicesToExplain & choices_to_explain,
            AlreadyCycleNotes & already_cycle_notes)
    {
        if (! resolved->untaken_change_or_remove_decisions()->empty())
            display_a_changes_and_removes(env, resolved, resolved->untaken_change_or_remove_decisions(),
                    cmdline, choices_to_explain, true, false, true, false, make_null_shared_ptr(), already_cycle_notes);
    }

    void display_an_errors(
            const std::shared_ptr<Environment> & env,
            const std::shared_ptr<const Resolved> & resolved,
            const std::shared_ptr<const Decisions<UnableToMakeDecision> > & decisions,
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
            const std::shared_ptr<Environment> & env,
            const std::shared_ptr<const Resolved> & resolved,
            const DisplayResolutionCommandLine & cmdline)
    {
        if (! resolved->taken_unable_to_make_decisions()->empty())
            display_an_errors(env, resolved, resolved->taken_unable_to_make_decisions(), cmdline, false);
    }

    void display_untaken_errors(
            const std::shared_ptr<Environment> & env,
            const std::shared_ptr<const Resolved> & resolved,
            const DisplayResolutionCommandLine & cmdline)
    {
        if (! resolved->untaken_unable_to_make_decisions()->empty())
            display_an_errors(env, resolved, resolved->untaken_unable_to_make_decisions(), cmdline, true);
    }

    void display_taken_changes_requiring_confirmation(
            const std::shared_ptr<Environment> & env,
            const std::shared_ptr<const Resolved> & resolved,
            const DisplayResolutionCommandLine & cmdline,
            AlreadyCycleNotes & already_cycle_notes)
    {
        ChoicesToExplain ignore_choices_to_explain;
        if (! resolved->taken_unconfirmed_decisions()->empty())
            display_a_changes_and_removes(env, resolved, resolved->taken_unconfirmed_decisions(),
                    cmdline, ignore_choices_to_explain, true, true, false, false, make_null_shared_ptr(), already_cycle_notes);
    }
}

bool
DisplayResolutionCommand::important() const
{
    return false;
}

int
DisplayResolutionCommand::run(
        const std::shared_ptr<Environment> & env,
        const std::shared_ptr<const Sequence<std::string > > & args,
        const std::shared_ptr<const Resolved> & maybe_resolved
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

    std::shared_ptr<const Resolved> resolved(maybe_resolved);
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
    AlreadyCycleNotes already_cycle_notes;
    auto totals(std::make_shared<Totals>());
    display_changes_and_removes(env, resolved, cmdline, choices_to_explain, totals, already_cycle_notes);
    display_totals(totals);
    display_unorderable_changes_and_removed(env, resolved, cmdline, choices_to_explain, already_cycle_notes);
    display_untaken_changes_and_removes(env, resolved, cmdline, choices_to_explain, already_cycle_notes);
    display_choices_to_explain(env, cmdline, choices_to_explain);
    display_taken_errors(env, resolved, cmdline);
    display_untaken_errors(env, resolved, cmdline);
    display_taken_changes_requiring_confirmation(env, resolved, cmdline, already_cycle_notes);
    display_explanations(env, resolved, cmdline);

    return 0;
}

int
DisplayResolutionCommand::run(
        const std::shared_ptr<Environment> & env,
        const std::shared_ptr<const Sequence<std::string > > & args)
{
    return run(env, args, make_null_shared_ptr());
}

std::shared_ptr<args::ArgsHandler>
DisplayResolutionCommand::make_doc_cmdline()
{
    return std::make_shared<DisplayResolutionCommandLine>();
}

