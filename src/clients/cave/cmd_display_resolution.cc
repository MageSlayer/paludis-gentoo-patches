/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010, 2011, 2012, 2013 Ciaran McCreesh
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
#include "resolve_cmdline.hh"
#include "exceptions.hh"
#include "command_command_line.hh"
#include "colours.hh"
#include "colour_pretty_printer.hh"
#include "format_user_config.hh"
#include "parse_spec_with_nice_error.hh"

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
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/log.hh>
#include <paludis/util/pretty_print.hh>
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
#include <paludis/resolver/why_changed_choices.hh>
#include <paludis/resolver/collect_depped_upon.hh>

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
#include <paludis/dep_spec_annotations.hh>
#include <paludis/slot.hh>

#include <algorithm>
#include <set>
#include <iterator>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <map>
#include <limits>
#include <unistd.h>

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
#include "cmd_display_resolution-fmt.hh"

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

    WantPhase want_all_phases(const std::string &)
    {
        return wp_yes;
    }

    std::string get_annotation(
            const std::shared_ptr<const DepSpecAnnotations> & annotations,
            const DepSpecAnnotationRole role)
    {
        auto i(annotations->find(role));
        if (i == annotations->end())
            return "";

        return i->value();
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
                const std::shared_ptr<const DepSpecAnnotations> & key,
                const std::pair<std::string, Tribool> unannotated,
                const bool annotate_regardless) const
        {
            if ((! key) || ((! annotate_regardless) && (! more_annotations)))
                return unannotated;

            std::pair<std::string, Tribool> result(unannotated);

            std::string description_annotation(get_annotation(key, dsar_general_description));
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
                return annotate(r.sanitised_dependency().spec().if_block()->maybe_annotations(),
                        std::make_pair(stringify(*r.sanitised_dependency().spec().if_block())
                            + " from " + stringify(*r.from_id()),
                            false), true);
            else
            {
                if (verbose)
                {
                    std::string as;
                    if (! r.sanitised_dependency().original_specs_as_string().empty())
                        as = " (originally " + r.sanitised_dependency().original_specs_as_string() + ")";

                    return annotate(r.sanitised_dependency().spec().if_package()->maybe_annotations(),
                            std::make_pair(stringify(*r.sanitised_dependency().spec().if_package())
                                + " from " + stringify(*r.from_id()) + ", key '"
                                + r.sanitised_dependency().metadata_key_human_name() + "'"
                                + (r.sanitised_dependency().active_dependency_labels_as_string().empty() ? "" :
                                    ", labelled '" + r.sanitised_dependency().active_dependency_labels_as_string() + "'")
                                + (r.sanitised_dependency().active_conditions_as_string().empty() ? "" :
                                    ", conditions '" + r.sanitised_dependency().active_conditions_as_string() + "'")
                                + as,
                                false), false);
                }
                else
                {
                    std::string ts;
                    const auto & cs(*r.sanitised_dependency().active_dependency_labels_classifier());
                    if (cs.includes_buildish && ! cs.includes_non_post_runish && ! cs.includes_post &&
                            ! cs.includes_non_test_buildish)
                        ts.append(" (test)");

                    if (! cs.is_requirement)
                    {
                        if (cs.is_recommendation)
                            ts.append(" (rec)");
                        else if (cs.is_suggestion)
                            ts.append(" (sug)");
                    }

                    return annotate(r.sanitised_dependency().spec().if_package()->maybe_annotations(),
                            std::make_pair(stringify(*r.from_id()) + ts, false), false);
                }
            }
        }

        std::pair<std::string, Tribool> visit(const DependentReason & r) const
        {
            return std::make_pair("dependent upon " + stringify(*r.dependent_upon().package_id())
                    + " (" + r.dependent_upon().active_dependency_labels_as_string() + ")", true);
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

    const std::shared_ptr<const PackageID> id_for_decision_or_null(const Decision & decision)
    {
        return decision.make_accept_returning(
                [&] (const ExistingNoChangeDecision & d) { return d.existing_id(); },
                [&] (const BreakDecision & d)            { return d.existing_id(); },
                [&] (const ChangesToMakeDecision & d)    { return d.origin_id(); },
                [&] (const UnableToMakeDecision &)       { return nullptr; },
                [&] (const RemoveDecision &)             { return nullptr; },
                [&] (const NothingNoChangeDecision &)    { return nullptr; }
                );
    }

    bool decision_matches_spec(
            const std::shared_ptr<Environment> & env,
            const Resolvent & resolvent,
            const Decision & decision,
            const PackageDepSpec & spec)
    {
        const std::shared_ptr<const PackageID> maybe_id(id_for_decision_or_null(decision));
        if (maybe_id)
            return match_package(*env, spec, maybe_id, nullptr, { });
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
            case ue_if_same_metadata:
                result << ", use existing if same metadata";
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

        if (c.nothing_is_fine_too())
            result << " (nothing is fine too)";

        return result.str();
    }

    void display_explanation_constraints(const Constraints & constraints)
    {
        std::map<std::string, std::set<std::string> > reasons_for_constraints;
        for (Constraints::ConstIterator c(constraints.begin()), c_end(constraints.end()) ;
                c != c_end ; ++c)
        {
            auto & reasons(reasons_for_constraints.insert(std::make_pair(
                            constraint_as_string(**c), std::set<std::string>())).first->second);

            ReasonNameGetter g(true, true);
            reasons.insert((*c)->reason()->accept_returning<std::pair<std::string, Tribool> >(g).first);
        }

        cout << fuc(fs_explanation_constraints_header());
        for (auto c(reasons_for_constraints.begin()), c_end(reasons_for_constraints.end()) ;
                c != c_end ; ++c)
        {
            cout << fuc(fs_explanation_constraint(), fv<'c'>(c->first));
            for (auto r(c->second.begin()), r_end(c->second.end()) ;
                    r != r_end ; ++r)
                cout << fuc(fs_explanation_constraint_reason(), fv<'r'>(*r));
        }
    }

    void display_explanation_decision(const Decision & decision)
    {
        decision.make_accept(
                [&] (const ExistingNoChangeDecision & d) {
                    cout << fuc(fs_explanation_decision_heading());
                    if (d.taken())
                        cout << fuc(fs_explanation_decision_existing_taken(), fv<'i'>(stringify(*d.existing_id())));
                    else
                        cout << fuc(fs_explanation_decision_existing_untaken(), fv<'i'>(stringify(*d.existing_id())));
                },

                [&] (const RemoveDecision & d) {
                    cout << fuc(fs_explanation_decision_heading());
                    if (d.taken())
                        cout << fuc(fs_explanation_decision_remove_taken());
                    else
                        cout << fuc(fs_explanation_decision_remove_untaken());

                    for (PackageIDSequence::ConstIterator i(d.ids()->begin()), i_end(d.ids()->end()) ;
                            i != i_end ; ++i)
                        cout << fuc(fs_explanation_decision_remove_id(), fv<'i'>(stringify(**i)));
                },

                [&] (const NothingNoChangeDecision &) {
                    cout << fuc(fs_explanation_decision_heading());
                    cout << fuc(fs_explanation_decision_nothing());
                },

                [&] (const ChangesToMakeDecision & d) {
                    if (d.taken())
                        cout << fuc(fs_explanation_decision_heading());
                    else
                        cout << fuc(fs_explanation_decision_untaken_heading());

                    cout << fuc(fs_explanation_decision_change_origin(), fv<'i'>(stringify(*d.origin_id())));
                    if (d.if_via_new_binary_in())
                        cout << fuc(fs_explanation_decision_change_via(), fv<'r'>(stringify(*d.if_via_new_binary_in())));
                    cout << fuc(fs_explanation_decision_change_destination(), fv<'r'>(stringify(d.destination()->repository())));

                    for (PackageIDSequence::ConstIterator i(d.destination()->replacing()->begin()), i_end(d.destination()->replacing()->end()) ;
                            i != i_end ; ++i)
                        cout << fuc(fs_explanation_decision_change_replacing(), fv<'i'>(stringify(**i)));
                },

                [&] (const UnableToMakeDecision & d) {
                    if (d.taken())
                        cout << fuc(fs_explanation_decision_unable_taken());
                    else
                        cout << fuc(fs_explanation_decision_unable_untaken());
                },

                [&] (const BreakDecision & d) {
                    if (d.taken())
                        cout << fuc(fs_explanation_decision_break_taken(), fv<'i'>(stringify(*d.existing_id())));
                    else
                        cout << fuc(fs_explanation_decision_break_untaken(), fv<'i'>(stringify(*d.existing_id())));
                }
                );
    }

    void display_explanations(
            const std::shared_ptr<Environment> & env,
            const std::shared_ptr<const Resolved> & resolved,
            const DisplayResolutionCommandLine & cmdline)
    {
        Context context("When displaying explanations:");

        if (cmdline.display_options.a_explain.begin_args() == cmdline.display_options.a_explain.end_args())
            return;

        cout << fuc(fs_explaining());

        for (args::StringSetArg::ConstIterator i(cmdline.display_options.a_explain.begin_args()),
                i_end(cmdline.display_options.a_explain.end_args()) ;
                i != i_end ; ++i)
        {
            bool any(false);
            PackageDepSpec spec(parse_spec_with_nice_error(*i, env.get(), { updso_allow_wildcards }, filter::All()));
            for (ResolutionsByResolvent::ConstIterator r(resolved->resolutions_by_resolvent()->begin()),
                    r_end(resolved->resolutions_by_resolvent()->end()) ;
                    r != r_end ; ++r)
            {
                if (! decision_matches_spec(env, (*r)->resolvent(), *(*r)->decision(), spec))
                    continue;

                any = true;

                cout << fuc(fs_explaining_resolvent(), fv<'r'>(stringify((*r)->resolvent())));

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
        if (id->short_description_key() && ! id->short_description_key()->parse_value().empty())
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
                cout << fuc(fs_description(), fv<'s'>(id->short_description_key()->parse_value()));
        }
    }

    bool show_choice_value_even_if_hidden(
            const std::shared_ptr<const ChoiceValue> & v,
            const std::shared_ptr<const Choices> & old_choices)
    {
        if (! old_choices)
            return false;

        switch (v->origin())
        {
            case co_implicit: return false;
            case co_special:  return false;
            case co_explicit: break;
            case last_co:     break;
        }

        std::shared_ptr<const ChoiceValue> old_choice(old_choices->find_by_name_with_prefix(v->name_with_prefix()));
        if (! old_choice)
            return true;
        else if (old_choice->enabled() != v->enabled())
            return true;

        return false;
    }

    void display_choices(
            const std::shared_ptr<Environment> & env,
            const DisplayResolutionCommandLine & cmdline,
            const std::shared_ptr<const PackageID> & id,
            const std::shared_ptr<const WhyChangedChoices> & changed_choices,
            const std::shared_ptr<const PackageID> & old_id,
            ChoicesToExplain & choices_to_explain
            )
    {
        if (! id->choices_key())
            return;

        ColourPrettyPrinter printer(env.get(), id, 0);

        std::shared_ptr<const Choices> old_choices;
        if (old_id && old_id->choices_key())
            old_choices = old_id->choices_key()->parse_value();

        std::pair<std::string, bool> changed_s_prefix("", false), unchanged_s_prefix("", false);
        auto choices(id->choices_key()->parse_value());
        for (Choices::ConstIterator k(choices->begin()), k_end(choices->end()) ;
                k != k_end ; ++k)
        {
            if ((*k)->hidden() && (*k)->consider_added_or_changed() && old_choices)
            {
                /* ignore the hide if anything has changed */
                bool show_anyway(false);
                for (Choice::ConstIterator i((*k)->begin()), i_end((*k)->end()) ;
                        i != i_end && ! show_anyway ; ++i)
                    if (show_choice_value_even_if_hidden(*i, old_choices))
                        show_anyway = true;

                if (! show_anyway)
                    continue;
            }

            if ((*k)->hide_description())
                if (std::none_of((*k)->begin(), (*k)->end(),
                                 [&old_choices](const std::shared_ptr<const ChoiceValue> & value) {
                                     return show_choice_value_even_if_hidden(value, old_choices);
                                 }))
                    continue;

            bool shown_prefix_changed(false), shown_prefix_unchanged(false);
            for (Choice::ConstIterator i((*k)->begin()), i_end((*k)->end()) ;
                    i != i_end ; ++i)
            {
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

                if (co_implicit == (*i)->origin() || (*k)->hidden())
                {
                    if (added || changed)
                    {
                        if (! show_choice_value_even_if_hidden(*i, old_choices))
                            continue;
                    }
                    else
                        continue;
                }

                Tribool changed_state(indeterminate);
                if (changed_choices)
                    changed_state = changed_choices->changed_choices()->overridden_value((*i)->name_with_prefix());

                auto & s_prefix(changed_state.is_indeterminate() ? unchanged_s_prefix : changed_s_prefix);
                auto & shown_prefix(changed_state.is_indeterminate() ? shown_prefix_unchanged : shown_prefix_changed);

                if (! shown_prefix)
                {
                    if (s_prefix.second || ! (*k)->show_with_no_prefix())
                    {
                        s_prefix.second = true;
                        shown_prefix = true;
                        if (! s_prefix.first.empty())
                            s_prefix.first.append(" ");
                        s_prefix.first.append((*k)->raw_name() + ":");
                    }
                }

                if (! s_prefix.first.empty())
                    s_prefix.first.append(" ");

                std::string t;

                if (! changed_state.is_indeterminate())
                {
                    if ((*i)->enabled())
                        t = t + printer.prettify_choice_value_enabled(*i);
                    else
                        t = t + printer.prettify_choice_value_disabled(*i);
                    t = t + " -> ";
                }

                if ((changed_state.is_indeterminate() && (*i)->enabled()) || (changed_state.is_true()))
                {
                    if ((*i)->locked())
                        t = t + printer.prettify_choice_value_forced(*i);
                    else
                        t = t + printer.prettify_choice_value_enabled(*i);
                }
                else
                {
                    if ((*i)->locked())
                        t = t + printer.prettify_choice_value_masked(*i);
                    else
                        t = t + printer.prettify_choice_value_disabled(*i);
                }

                if (changed)
                    t = t + "*";
                else if (added)
                {
                    if (old_id)
                        t = t + "+";
                }

                s_prefix.first.append(t);

                bool show_description;
                if (cmdline.display_options.a_show_option_descriptions.argument() == "none")
                    show_description = false;
                else if (cmdline.display_options.a_show_option_descriptions.argument() == "new")
                    show_description = added || ! old_id;
                else if (cmdline.display_options.a_show_option_descriptions.argument() == "changed")
                    show_description = added || changed || ! old_id;
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

        if (changed_s_prefix.first.empty() && unchanged_s_prefix.first.empty())
            return;

        if (! changed_s_prefix.first.empty())
            cout << fuc(fs_choices_need_changes(), fv<'c'>(changed_s_prefix.first), fv<'u'>(unchanged_s_prefix.first));
        else
            cout << fuc(fs_choices(), fv<'u'>(unchanged_s_prefix.first));
    }

    void display_reasons(
            const std::shared_ptr<const Resolution> & resolution,
            const std::shared_ptr<const WhyChangedChoices> & maybe_changed_choices,
            const bool more_annotations
            )
    {
        std::set<std::string> reasons, special_reasons, changes_reasons;

        if (maybe_changed_choices)
            for (auto c(maybe_changed_choices->reasons()->begin()), c_end(maybe_changed_choices->reasons()->end()) ;
                    c != c_end ; ++c)
            {
                ReasonNameGetter g(false, more_annotations);
                std::pair<std::string, Tribool> r((*c)->accept_returning<std::pair<std::string, Tribool> >(g));
                if (r.first.empty())
                    continue;
                changes_reasons.insert(r.first);
            }

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

        for (std::set<std::string>::const_iterator r(changes_reasons.begin()), r_end(changes_reasons.end()) ;
                r != r_end ; ++r)
        {
            special_reasons.erase(*r);
            reasons.erase(*r);
        }

        for (std::set<std::string>::const_iterator r(special_reasons.begin()), r_end(special_reasons.end()) ;
                r != r_end ; ++r)
            reasons.erase(*r);

        if (reasons.empty() && special_reasons.empty() && changes_reasons.empty())
            return;

        cout << fuc(fs_reasons_start());

        int n_shown(0);

        if (! changes_reasons.empty())
        {
            cout << fuc(fs_changes_reasons_start());
            for (std::set<std::string>::const_iterator r(changes_reasons.begin()), r_end(changes_reasons.end()) ;
                    r != r_end ; ++r)
                cout << fuc(fs_reason_changes(), fv<'c'>(++n_shown != 1 ? ", " : ""), fv<'r'>(*r));
            cout << fuc(fs_changes_reasons_end());
        }

        if ((! special_reasons.empty()) || (! reasons.empty()))
            cout << fuc(fs_reasons());
        n_shown = 0;

        for (std::set<std::string>::const_iterator r(special_reasons.begin()), r_end(special_reasons.end()) ;
                r != r_end ; ++r)
            cout << fuc(fs_reason_special(), fv<'c'>(++n_shown != 1 ? ", " : ""), fv<'r'>(*r));

        int n_remaining(reasons.size());
        for (std::set<std::string>::const_iterator r(reasons.begin()), r_end(reasons.end()) ;
                r != r_end ; ++r)
        {
            if (n_shown >= 3 && n_remaining > 1)
            {
                cout << fuc(fs_reason_n_more(), fv<'c'>(", "), fv<'n'>(stringify(n_remaining)));
                break;
            }

            --n_remaining;
            cout << fuc(fs_reason_normal(), fv<'c'>(++n_shown != 1 ? ", " : ""), fv<'r'>(*r));
        }

        cout << fuc(fs_reasons_end());
    }

    std::string stringify_confirmation(const RequiredConfirmation & c)
    {
        return c.make_accept_returning(
                [&] (const DowngradeConfirmation &)           { return "--permit-downgrade"; },
                [&] (const NotBestConfirmation &)             { return "--permit-old-version"; },
                [&] (const BreakConfirmation &)               { return "--uninstalls-may-break or --remove-if-dependent"; },
                [&] (const RemoveSystemPackageConfirmation &) { return "--uninstalls-may-break system"; },
                [&] (const UninstallConfirmation &)           { return "--permit-uninstall (check this very carefully!)"; },
                [&] (const MaskedConfirmation &)              { return "being unmasked"; },
                [&] (const ChangedChoicesConfirmation &)      { return "user configuration changes"; }
            );
    }

    void display_confirmations(
            const ConfirmableDecision & decision)
    {
        const std::shared_ptr<const RequiredConfirmations> r(decision.required_confirmations_if_any());
        if (r && ! r->empty())
            cout << fuc(fs_confirm(), fv<'s'>(join(indirect_iterator(r->begin()), indirect_iterator(r->end()), ", ", stringify_confirmation)));
    }

    std::string find_suggestion_groups(
            const std::shared_ptr<const Resolution> & resolution,
            const Decision &)
    {
        std::stringstream result;

        for (auto c(resolution->constraints()->begin()), c_end(resolution->constraints()->end()) ;
                c != c_end ; ++c)
        {
            const DepSpec & spec((*c)->spec().if_block() ? static_cast<const DepSpec &>(*(*c)->spec().if_block()) : *(*c)->spec().if_package());
            if (spec.maybe_annotations())
            {
                auto a(spec.maybe_annotations()->find(dsar_suggestions_group_name));
                if (a != spec.maybe_annotations()->end())
                {
                    if (! result.str().empty())
                        result << ", ";
                    result << a->value();
                }
            }
        }

        return result.str();
    }

    void display_untaken_change(
            const std::shared_ptr<const Resolution> resolution,
            const ChangesToMakeDecision & decision)
    {
        cout << fuc(fs_take(), fv<'g'>(find_suggestion_groups(resolution, decision)));
    }

    void display_untaken_remove(const RemoveDecision &)
    {
        cout << fuc(fs_take_purge());
    }

    struct MaskedByKeyVisitor
    {
        const Environment * const env;
        const std::shared_ptr<const PackageID> id;
        const std::string indent;

        void visit(const MetadataValueKey<std::shared_ptr<const PackageID> > & k)
        {
            cout << fuc(fs_mask_by(), fv<'i'>(indent), fv<'k'>(k.human_name()), fv<'v'>(stringify(*k.parse_value())));
        }

        void visit(const MetadataValueKey<std::string> & k)
        {
            cout << fuc(fs_mask_by(), fv<'i'>(indent), fv<'k'>(k.human_name()), fv<'v'>(stringify(k.parse_value())));
        }

        void visit(const MetadataValueKey<Slot> & k)
        {
            cout << fuc(fs_mask_by(), fv<'i'>(indent), fv<'k'>(k.human_name()), fv<'v'>(stringify(k.parse_value().raw_value())));
        }

        void visit(const MetadataValueKey<long> & k)
        {
            cout << fuc(fs_mask_by(), fv<'i'>(indent), fv<'k'>(k.human_name()), fv<'v'>(stringify(k.parse_value())));
        }

        void visit(const MetadataValueKey<bool> & k)
        {
            cout << fuc(fs_mask_by(), fv<'i'>(indent), fv<'k'>(k.human_name()), fv<'v'>(stringify(k.parse_value())));
        }

        void visit(const MetadataSectionKey & k)
        {
            cout << fuc(fs_mask_by_valueless(), fv<'i'>(indent), fv<'k'>(k.human_name()));
        }

        void visit(const MetadataTimeKey & k)
        {
            cout << fuc(fs_mask_by(), fv<'i'>(indent), fv<'k'>(k.human_name()), fv<'v'>(pretty_print_time(k.parse_value().seconds())));
        }

        void visit(const MetadataValueKey<FSPath> & k)
        {
            cout << fuc(fs_mask_by(), fv<'i'>(indent), fv<'k'>(k.human_name()), fv<'v'>(stringify(k.parse_value())));
        }

        void visit(const MetadataCollectionKey<Set<std::string> > & k)
        {
            ColourPrettyPrinter printer(env, id, 0);
            cout << fuc(fs_mask_by(), fv<'i'>(indent), fv<'k'>(k.human_name()), fv<'v'>(stringify(k.pretty_print_value(printer, { }))));
        }

        void visit(const MetadataCollectionKey<Map<std::string, std::string> > & k)
        {
            ColourPrettyPrinter printer(env, id, 0);
            cout << fuc(fs_mask_by(), fv<'i'>(indent), fv<'k'>(k.human_name()), fv<'v'>(stringify(k.pretty_print_value(printer, { }))));
        }

        void visit(const MetadataCollectionKey<Sequence<std::string> > & k)
        {
            ColourPrettyPrinter printer(env, id, 0);
            cout << fuc(fs_mask_by(), fv<'i'>(indent), fv<'k'>(k.human_name()), fv<'v'>(stringify(k.pretty_print_value(printer, { }))));
        }

        void visit(const MetadataCollectionKey<Maintainers> & k)
        {
            ColourPrettyPrinter printer(env, id, 0);
            cout << fuc(fs_mask_by(), fv<'i'>(indent), fv<'k'>(k.human_name()), fv<'v'>(stringify(k.pretty_print_value(printer, { }))));
        }

        void visit(const MetadataCollectionKey<FSPathSequence> & k)
        {
            ColourPrettyPrinter printer(env, id, 0);
            cout << fuc(fs_mask_by(), fv<'i'>(indent), fv<'k'>(k.human_name()), fv<'v'>(stringify(k.pretty_print_value(printer, { }))));
        }

        void visit(const MetadataCollectionKey<PackageIDSequence> & k)
        {
            ColourPrettyPrinter printer(env, id, 0);
            cout << fuc(fs_mask_by(), fv<'i'>(indent), fv<'k'>(k.human_name()), fv<'v'>(stringify(k.pretty_print_value(printer, { }))));
        }

        void visit(const MetadataSpecTreeKey<FetchableURISpecTree> & k)
        {
            ColourPrettyPrinter printer(env, id, 0);
            cout << fuc(fs_mask_by(), fv<'i'>(indent), fv<'k'>(k.human_name()), fv<'v'>(stringify(k.pretty_print_value(printer, { }))));
        }

        void visit(const MetadataSpecTreeKey<SimpleURISpecTree> & k)
        {
            ColourPrettyPrinter printer(env, id, 0);
            cout << fuc(fs_mask_by(), fv<'i'>(indent), fv<'k'>(k.human_name()), fv<'v'>(stringify(k.pretty_print_value(printer, { }))));
        }

        void visit(const MetadataSpecTreeKey<LicenseSpecTree> & k)
        {
            ColourPrettyPrinter printer(env, id, 0);
            cout << fuc(fs_mask_by(), fv<'i'>(indent), fv<'k'>(k.human_name()), fv<'v'>(stringify(k.pretty_print_value(printer, { }))));
        }

        void visit(const MetadataSpecTreeKey<DependencySpecTree> & k)
        {
            ColourPrettyPrinter printer(env, id, 0);
            cout << fuc(fs_mask_by(), fv<'i'>(indent), fv<'k'>(k.human_name()), fv<'v'>(stringify(k.pretty_print_value(printer, { }))));
        }

        void visit(const MetadataCollectionKey<KeywordNameSet> & k)
        {
            ColourPrettyPrinter printer(env, id, 0);
            cout << fuc(fs_mask_by(), fv<'i'>(indent), fv<'k'>(k.human_name()), fv<'v'>(stringify(k.pretty_print_value(printer, { }))));
        }

        void visit(const MetadataSpecTreeKey<PlainTextSpecTree> & k)
        {
            ColourPrettyPrinter printer(env, id, 0);
            cout << fuc(fs_mask_by(), fv<'i'>(indent), fv<'k'>(k.human_name()), fv<'v'>(stringify(k.pretty_print_value(printer, { }))));
        }

        void visit(const MetadataSpecTreeKey<RequiredUseSpecTree> & k)
        {
            ColourPrettyPrinter printer(env, id, 0);
            cout << fuc(fs_mask_by(), fv<'i'>(indent), fv<'k'>(k.human_name()), fv<'v'>(stringify(k.pretty_print_value(printer, { }))));
        }

        void visit(const MetadataValueKey<std::shared_ptr<const Choices> > & k)
        {
            ColourPrettyPrinter printer(env, id, 0);
            cout << fuc(fs_mask_by_valueless(), fv<'i'>(indent), fv<'k'>(k.human_name()));
        }
    };

    struct MaskedByVisitor
    {
        const Environment * const env;
        const std::shared_ptr<const PackageID> id;
        const std::string colour;
        const std::string indent;

        void visit(const UserMask & m) const
        {
            cout << fuc(fs_masked_by(), fv<'i'>(indent), fv<'c'>(colour), fv<'d'>(m.description()), fv<'t'>(m.token()));
        }

        void visit(const RepositoryMask & m) const
        {
            cout << fuc(fs_masked_by(), fv<'i'>(indent), fv<'c'>(colour), fv<'d'>(m.description()), fv<'t'>(m.token()));
            if (! m.comment().empty())
                cout << fuc(fs_mask_by_repo_line(), fv<'i'>(indent + "    "), fv<'s'>(m.comment()));
        }

        void visit(const UnacceptedMask & m) const
        {
            cout << fuc(fs_masked_by(), fv<'i'>(indent), fv<'c'>(colour), fv<'d'>(m.description()), fv<'t'>(""));
            MaskedByKeyVisitor v{env, id, indent + "    "};
            if (! m.unaccepted_key_name().empty())
                (*id->find_metadata(m.unaccepted_key_name()))->accept(v);
        }

        void visit(const UnsupportedMask & m) const
        {
            cout << fuc(fs_masked_by_explanation(), fv<'i'>(indent), fv<'c'>(colour), fv<'d'>(m.description()), fv<'x'>(m.explanation()));
        }
    };

    void display_masks(
            const std::shared_ptr<Environment> & env,
            const ChangesToMakeDecision & decision)
    {
        if (! decision.origin_id()->masked())
            return;

        for (auto m(decision.origin_id()->begin_masks()), m_end(decision.origin_id()->end_masks()) ;
                m != m_end ; ++m)
            (*m)->accept(MaskedByVisitor{env.get(), decision.origin_id(), c::bold_red().colour_string(), "    "});
    }

    struct Totals
    {
        std::set<FSPath, FSPathComparator> download_files;
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

        void will_fetch(const FSPath & destination, const unsigned long size_in_bytes)
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
                    n::want_phase() = &want_all_phases
                    ),
                totals);
        id->perform_action(action);

        if (output_manager_holder.output_manager_if_constructed())
            output_manager_holder.output_manager_if_constructed()->succeeded();

        if (0 == action.size)
            return;

        if (action.overflow)
            cout << fuc(fs_download_megalots(), fv<'i'>(pretty_print_bytes(std::numeric_limits<unsigned long>::max())));
        else
            cout << fuc(fs_download_amount(), fv<'i'>(pretty_print_bytes(action.size)));

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
            c = c::red().colour_string();
        else
            do
            {
                switch (resolution->resolvent().destination_type())
                {
                    case dt_install_to_slash:
                        if (decision.if_via_new_binary_in())
                            c = c::yellow().colour_string();
                        if (maybe_totals)
                            ++maybe_totals->installs_ct_count.insert(std::make_pair(decision.change_type(), 0)).first->second;
                        continue;

                    case dt_install_to_chroot:
                        c = c::blue().colour_string();
                        if (maybe_totals)
                            ++maybe_totals->installs_ct_count.insert(std::make_pair(decision.change_type(), 0)).first->second;
                        continue;

                    case dt_create_binary:
                        c = c::green().colour_string();
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
                    cout << fuc(fs_change_type_new(), fv<'c'>(c), fv<'s'>(x.replace(x.find('X'), 1, "n")));
                    continue;
                case ct_slot_new:
                    cout << fuc(fs_change_type_slot_new(), fv<'c'>(c), fv<'s'>(x.replace(x.find('X'), 1, "s")));
                    continue;
                case ct_add_to_slot:
                    cout << fuc(fs_change_type_add_to_slot(), fv<'c'>(c), fv<'s'>(x.replace(x.find('X'), 1, "v")));
                    continue;
                case ct_upgrade:
                    cout << fuc(fs_change_type_upgrade(), fv<'c'>(c), fv<'s'>(x.replace(x.find('X'), 1, "u")));
                    continue;
                case ct_reinstall:
                    cout << fuc(fs_change_type_reinstall(), fv<'c'>(c), fv<'s'>(x.replace(x.find('X'), 1, "r")));
                    continue;
                case ct_downgrade:
                    cout << fuc(fs_change_type_downgrade(), fv<'c'>(c), fv<'s'>(x.replace(x.find('X'), 1, "d")));
                    continue;
                case last_ct:
                    break;
            }
            throw InternalError(PALUDIS_HERE, "bad change_type. huh?");
        }
        while (false);

        cout << fuc(fs_change_name(), fv<'i'>(decision.origin_id()->canonical_form(idcf_no_version)));

        if (! decision.best())
            cout << fuc(fs_change_not_best());

        if ((! decision.destination()->replacing()->empty()) &&
                (*decision.destination()->replacing()->begin())->from_repositories_key())
        {
            auto from_repositories((*decision.destination()->replacing()->begin())->from_repositories_key()->parse_value());
            if (! from_repositories->empty() && from_repositories->end() == from_repositories->find(stringify(decision.origin_id()->repository_name())))
            {
                cout << fuc(fs_change_formerly_from(), fv<'r'>(join(from_repositories->begin(), from_repositories->end(), ", ::")));
            }
        }

        cout << fuc(fs_change_version(), fv<'v'>(decision.origin_id()->canonical_form(idcf_version)));
        cout << fuc(fs_change_destination(), fv<'r'>(stringify(decision.destination()->repository())));

        if (decision.if_via_new_binary_in())
            cout << fuc(fs_change_via(), fv<'r'>(stringify(*decision.if_via_new_binary_in())));

        if (! decision.destination()->replacing()->empty())
        {
            cout << fuc(fs_change_replacing());
            bool first(true);
            for (PackageIDSequence::ConstIterator i(decision.destination()->replacing()->begin()),
                    i_end(decision.destination()->replacing()->end()) ;
                    i != i_end ; ++i)
            {
                std::string comma;
                if (! first)
                    comma = ", ";
                first = false;

                cout << fuc(fs_change_replacing_one(), fv<'c'>(comma), fv<'s'>(
                            (*i)->name() == decision.origin_id()->name() ? (*i)->canonical_form(idcf_version) : (*i)->canonical_form(idcf_full)));
            }
        }

        if (-1 != cycle_notes_heading)
            cout << fuc(unorderable ? fs_cycle_heading_error() : fs_cycle_heading(),
                    fv<'s'>(stringify(cycle_notes_heading)));

        cout << fuc(fs_change_end());

        std::shared_ptr<const PackageID> old_id;
        if (! decision.destination()->replacing()->empty())
            old_id = *decision.destination()->replacing()->begin();

        display_one_description(env, cmdline, decision.origin_id(), ! old_id);
        display_choices(env, cmdline, decision.origin_id(), decision.if_changed_choices(), old_id, choices_to_explain);
        display_reasons(resolution, decision.if_changed_choices(), more_annotations);
        display_masks(env, decision);
        if (maybe_totals)
            display_downloads(env, cmdline, decision.origin_id(), maybe_totals);
        if (untaken)
            display_untaken_change(resolution, decision);
        if (confirmations)
            display_confirmations(decision);
        if (! cycle_notes.empty())
            cout << fuc(unorderable ? fs_cycle_notes_error() : fs_cycle_notes(),
                    fv<'s'>(cycle_notes));
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
            cout << fuc(fs_uninstall_untaken(), fv<'s'>(stringify(decision.resolvent().package())));
        else
            cout << fuc(fs_uninstall_taken(), fv<'s'>(stringify(decision.resolvent().package())));

        bool first(true);
        for (PackageIDSequence::ConstIterator i(decision.ids()->begin()),
                i_end(decision.ids()->end()) ;
                i != i_end ; ++i)
        {
            std::string comma;
            if (! first)
                comma = ", ";
            first = false;

            cout << fuc(fs_uninstall_version(), fv<'c'>(comma), fv<'v'>(stringify((*i)->canonical_form(idcf_no_name))));
        }

        if (-1 != cycle_notes_heading)
            cout << fuc(unorderable ? fs_cycle_heading_error() : fs_cycle_heading(),
                    fv<'s'>(stringify(cycle_notes_heading)));

        cout << fuc(fs_uninstall_end());

        display_reasons(resolution, nullptr, more_annotations);
        if (untaken)
            display_untaken_remove(decision);
        if (confirmations)
            display_confirmations(decision);
        if (! cycle_notes.empty())
            cout << fuc(unorderable ? fs_cycle_notes_error() : fs_cycle_notes(),
                    fv<'s'>(cycle_notes));

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
            cout << fuc(fs_unable_untaken(), fv<'s'>(stringify(d.resolvent().package())));
        else
            cout << fuc(fs_unable_taken(), fv<'s'>(stringify(d.resolvent().package())));

        display_reasons(resolution, nullptr, true);

        if (untaken)
            return;

        cout << fuc(fs_unable_unsuitable_header());

        if (d.unsuitable_candidates()->empty())
        {
            cout << fuc(fs_unable_unsuitable_nothing(), fv<'r'>(stringify(resolution->resolvent())));
            return;
        }

        for (UnsuitableCandidates::ConstIterator u(d.unsuitable_candidates()->begin()),
                u_end(d.unsuitable_candidates()->end()) ;
                u != u_end ; ++u)
        {
            std::string colour;
            if (! u->unmet_constraints()->empty())
                colour = c::red().colour_string();
            else if (u->package_id()->masked())
            {
                if (not_strongly_masked(u->package_id()))
                    colour = c::bold_red().colour_string();
                else
                    colour = c::red().colour_string();
            }

            cout << fuc(fs_unable_unsuitable_id(), fv<'c'>(colour), fv<'i'>(stringify(*u->package_id())));

            for (PackageID::MasksConstIterator m(u->package_id()->begin_masks()),
                    m_end(u->package_id()->end_masks()) ;
                    m != m_end ; ++m)
                (*m)->accept(MaskedByVisitor{env.get(), u->package_id(), "", "        "});

            std::map<std::string, std::pair<std::shared_ptr<const Constraint>, std::set<std::string> > > duplicates;
            for (Constraints::ConstIterator c(u->unmet_constraints()->begin()),
                    c_end(u->unmet_constraints()->end()) ;
                    c != c_end ; ++c)
            {
                ReasonNameGetter g(false, true);
                duplicates.insert(std::make_pair(constraint_as_string(**c), std::make_pair(*c, std::set<std::string>()))).first->second.second.insert(
                        (*c)->reason()->accept_returning<std::pair<std::string, Tribool> >(g).first);
            }

            for (auto c(duplicates.begin()), c_end(duplicates.end()) ;
                    c != c_end ; ++c)
            {
                auto constraint(c->second.first);

                ReasonNameGetter g(false, true);
                std::string s(constraint_as_string(*constraint) + " from " +
                        constraint->reason()->accept_returning<std::pair<std::string, Tribool> >(g).first);

                if (c->second.second.size() > 1)
                    s.append(" (and " + stringify(c->second.second.size() - 1) + " more)");
                cout << fuc(fs_unable_unsuitable_did_not_meet(), fv<'s'>(s));

                if (constraint->spec().if_package() && constraint->spec().if_package()->additional_requirements_ptr() &&
                        (! match_package(*env, *constraint->spec().if_package(), u->package_id(), constraint->from_id(), { })) &&
                        match_package(*env, *constraint->spec().if_package(), u->package_id(), constraint->from_id(), { mpo_ignore_additional_requirements }))
                {
                    for (AdditionalPackageDepSpecRequirements::ConstIterator a(constraint->spec().if_package()->additional_requirements_ptr()->begin()),
                            a_end(constraint->spec().if_package()->additional_requirements_ptr()->end()) ;
                            a != a_end ; ++a)
                    {
                        const std::pair<bool, std::string> p((*a)->requirement_met(env.get(), 0, u->package_id(), constraint->from_id(), 0));
                        if (p.first)
                            continue;

                        cout << fuc(fs_unable_unsuitable_did_not_meet_additional(), fv<'s'>(p.second));
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
            cout << fuc(fs_choice_to_explain_prefix(), fv<'s'>(p->first));

            for (ChoiceValuesToExplain::const_iterator v(p->second.begin()), v_end(p->second.end()) ;
                    v != v_end ; ++v)
            {
                bool all_same(true);
                const std::shared_ptr<const ChoiceValue> first_choice_value(
                        (*v->second->begin())->choices_key()->parse_value()->find_by_name_with_prefix(v->first));
                std::string description(first_choice_value->description());
                for (PackageIDSequence::ConstIterator w(next(v->second->begin())), w_end(v->second->end()) ;
                        w != w_end ; ++w)
                    if ((*w)->choices_key()->parse_value()->find_by_name_with_prefix(v->first)->description() != description)
                    {
                        all_same = false;
                        break;
                    }

                if (all_same)
                    cout << fuc(fs_choice_to_explain_all_same(), fv<'s'>(stringify(first_choice_value->unprefixed_name())), fv<'d'>(description));
                else
                {
                    cout << fuc(fs_choice_to_explain_not_all_same(), fv<'s'>(stringify(first_choice_value->unprefixed_name())));
                    for (PackageIDSequence::ConstIterator w(v->second->begin()), w_end(v->second->end()) ;
                            w != w_end ; ++w)
                    {
                        const std::shared_ptr<const ChoiceValue> value(
                                (*w)->choices_key()->parse_value()->find_by_name_with_prefix(v->first));
                        cout << fuc(fs_choice_to_explain_one(), fv<'s'>((*w)->canonical_form(idcf_no_version)), fv<'d'>(value->description()));
                    }
                }
            }

            cout << endl;
        }
    }

    std::pair<std::shared_ptr<const ConfirmableDecision>, std::shared_ptr<const OrdererNotes> >
    get_decision_and_notes(const std::shared_ptr<const ConfirmableDecision> & d)
    {
        return std::make_pair(d, nullptr);
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
            cout << fuc(fs_break_untaken(), fv<'s'>(stringify(decision.resolvent().package())));
        else
            cout << fuc(fs_break_taken(), fv<'s'>(stringify(decision.resolvent().package())));

        cout << fuc(fs_break_id(), fv<'i'>(decision.existing_id()->canonical_form(idcf_no_name)));

        cout << fuc(fs_break_by());
        display_reasons(resolution, nullptr, more_annotations);
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
            cout << fuc(fs_display_untaken());
        else if (unconfirmed)
            cout << fuc(fs_display_unconfirmed());
        else if (unorderable)
            cout << fuc(fs_display_unorderable());
        else
            cout << fuc(fs_display_taken());

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

            cout << fuc(fs_display_one_done());
        }

        if (! any)
            cout << fuc(fs_nothing_to_do());

        cout << fuc(fs_display_done());
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

        cout << fuc(fs_totals_start());
        bool need_comma(false);

        for (EnumIterator<ChangeType> e, e_end(last_ct) ;
                e != e_end ; ++e)
        {
            auto c(totals->installs_ct_count.find(*e));
            if (c == totals->installs_ct_count.end())
                continue;

            std::string comma, kind;
            if (need_comma)
                comma = ", ";
            need_comma = true;

            switch (c->first)
            {
                case ct_new:
                    kind = "new installs";
                    break;

                case ct_slot_new:
                    kind = "new slot installs";
                    break;

                case ct_add_to_slot:
                    kind = "adding to slot";
                    break;

                case ct_upgrade:
                    kind = "upgrades";
                    break;

                case ct_reinstall:
                    kind = "reinstalls";
                    break;

                case ct_downgrade:
                    kind = "downgrades";
                    break;

                case last_ct:
                    throw InternalError(PALUDIS_HERE, "bad change_type. huh?");
            }

            cout << fuc(fs_totals_one(), fv<'c'>(comma), fv<'n'>(stringify(c->second)), fv<'k'>(kind));
        }

        std::string comma;
        if (0 != totals->binary_installs_count)
        {
            if (need_comma)
                comma = ", ";
            need_comma = true;
            cout << fuc(fs_totals_binaries(), fv<'c'>(comma), fv<'n'>(stringify(totals->binary_installs_count)));
        }

        if (0 != totals->uninstalls_count)
        {
            if (need_comma)
                comma = ", ";
            need_comma = true;
            cout << fuc(fs_totals_uninstalls(), fv<'c'>(comma), fv<'n'>(stringify(totals->uninstalls_count)));
        }

        if (totals->download_overflow)
            cout << fuc(fs_totals_download_megalots(), fv<'n'>(pretty_print_bytes(std::numeric_limits<unsigned long>::max())));
        else if (0 != totals->download_size)
            cout << fuc(fs_totals_download_amount(), fv<'n'>(pretty_print_bytes(totals->download_size)));

        cout << fuc(fs_totals_done());
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
                    cmdline, choices_to_explain, false, false, false, true, nullptr, already_cycle_notes);
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
                    cmdline, choices_to_explain, true, false, true, false, nullptr, already_cycle_notes);
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
            cout << fuc(fs_display_errors_untaken());
        else
            cout << fuc(fs_display_errors());

        for (Decisions<UnableToMakeDecision>::ConstIterator i(decisions->begin()),
                i_end(decisions->end()) ;
                i != i_end ; ++i)
        {
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
                    cmdline, ignore_choices_to_explain, true, true, false, false, nullptr, already_cycle_notes);
    }
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
    return run(env, args, nullptr);
}

std::shared_ptr<args::ArgsHandler>
DisplayResolutionCommand::make_doc_cmdline()
{
    return std::make_shared<DisplayResolutionCommandLine>();
}

CommandImportance
DisplayResolutionCommand::importance() const
{
    return ci_internal;
}

