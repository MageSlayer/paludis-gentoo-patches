/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009 Ciaran McCreesh
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
#include <paludis/resolver/resolutions.hh>
#include <paludis/resolver/serialise.hh>
#include <paludis/resolver/reason.hh>
#include <paludis/resolver/sanitised_dependencies.hh>
#include <paludis/resolver/resolution.hh>
#include <paludis/resolver/decision.hh>
#include <paludis/resolver/constraint.hh>
#include <paludis/resolver/resolver.hh>
#include <paludis/resolver/resolvent.hh>
#include <paludis/resolver/destination.hh>
#include <paludis/resolver/unsuitable_candidates.hh>
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

        DisplayResolutionCommandLine() :
            display_options(this)
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

    struct ReasonNameGetter
    {
        const bool verbose;

        ReasonNameGetter(const bool v) :
            verbose(v)
        {
        }

        std::pair<std::string, bool> visit(const DependencyReason & r) const
        {
            if (r.sanitised_dependency().spec().if_block())
                return std::make_pair(stringify(*r.sanitised_dependency().spec().if_block())
                        + " from " + (verbose ? stringify(*r.from_id()) : stringify(r.from_id()->name())),
                        true);
            else
            {
                if (verbose)
                {
                    std::string as;
                    if (! r.sanitised_dependency().original_specs_as_string().empty())
                        as = " (originally " + r.sanitised_dependency().original_specs_as_string() + ")";

                    return std::make_pair(stringify(*r.sanitised_dependency().spec().if_package())
                            + " from " + stringify(*r.from_id()) + ", key '"
                            + r.sanitised_dependency().metadata_key_human_name() + "'"
                            + (r.sanitised_dependency().active_dependency_labels_as_string().empty() ? "" :
                                ", labelled '" + r.sanitised_dependency().active_dependency_labels_as_string() + "'")
                            + as,
                            false);
                }
                else
                    return std::make_pair(stringify(r.from_id()->name()), false);
            }
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

        std::pair<std::string, bool> visit(const PresetReason &) const
        {
            return std::make_pair("", false);
        }
    };

    struct NotBestVisitor
    {
        bool visit(const ExistingNoChangeDecision &) const
        {
            return false;
        }

        bool visit(const NothingNoChangeDecision &) const
        {
            return false;
        }

        bool visit(const UnableToMakeDecision &) const
        {
            return false;
        }

        bool visit(const ChangesToMakeDecision & d) const
        {
            return ! d.best();
        }
    };

    void display_reasons(
            const std::tr1::shared_ptr<const Resolution> & resolution,
            const bool verbose)
    {
        std::set<std::string> reason_names, special_reason_names;
        for (Constraints::ConstIterator r(resolution->constraints()->begin()),
                r_end(resolution->constraints()->end()) ;
                r != r_end ; ++r)
        {
            ReasonNameGetter g(verbose);
            std::pair<std::string, bool> s((*r)->reason()->accept_returning<std::pair<std::string, bool> >(g));
            if (! s.first.empty())
            {
                if (s.second)
                    special_reason_names.insert(s.first);
                else
                    reason_names.insert(s.first);
            }
        }

        if ((! special_reason_names.empty()) || (! reason_names.empty()))
        {
            if (verbose)
            {
                cout << "    Because of" << endl;
                if (! special_reason_names.empty())
                    cout << "        * " << c::bold_yellow() << join(special_reason_names.begin(),
                            special_reason_names.end(), c::normal() + "\n        * " + c::bold_yellow())
                        << c::normal() << endl;

                if (! reason_names.empty())
                    cout << "        * " << join(reason_names.begin(), reason_names.end(), "\n        * ")
                        << endl;
            }
            else
            {
                cout << "    Because of ";

                if (! special_reason_names.empty())
                    cout << c::bold_yellow() << join(special_reason_names.begin(), special_reason_names.end(), ", ")
                        << c::normal();

                if (! reason_names.empty())
                {
                    if (! special_reason_names.empty())
                        cout << ", ";

                    if (reason_names.size() > 4)
                        cout << join(reason_names.begin(), next(reason_names.begin(), 3), ", ")
                            << ", " << (reason_names.size() - 3) << " more";
                    else
                        cout << join(reason_names.begin(), reason_names.end(), ", ");
                }

                cout << endl;
            }

            if (resolution->decision()->accept_returning<bool>(NotBestVisitor()))
                cout << c::bold_red() << "    Which prevented selection of the best candidate" << c::normal() << endl;
        }
    }

    std::string mask_stringifier(const Mask & mask)
    {
        return stringify(mask.key());
    }

    struct DisplayOneErrorVisitor
    {
        void visit(const UnableToMakeDecision & d) const
        {
            if (d.unsuitable_candidates()->empty())
                cout << "    No potential candidates were found" << endl;
            else
            {
                cout << "    Potential candidates were:" << endl;
                for (UnsuitableCandidates::ConstIterator u(d.unsuitable_candidates()->begin()), u_end(d.unsuitable_candidates()->end()) ;
                        u != u_end ; ++u)
                {
                    cout << "        " << *u->package_id() << ": ";

                    if (u->package_id()->masked())
                        cout << c::bold_red() << "masked" << c::normal() << " (" << join(indirect_iterator(u->package_id()->begin_masks()),
                                    indirect_iterator(u->package_id()->end_masks()), "", mask_stringifier) << ")";

                    if (! u->unmet_constraints()->empty())
                    {
                        if (u->package_id()->masked())
                            cout << ", ";
                        cout << c::bold_red() << "unmatching" << c::normal() << " (" << (*u->unmet_constraints()->begin())->spec();
                        int dx(std::distance(u->unmet_constraints()->begin(), u->unmet_constraints()->end()));
                        if (dx > 1)
                            cout << ", " << dx << " more";
                        cout << ")";
                    }

                    cout << endl;
                }
            }
        }

        void visit(const ExistingNoChangeDecision &) const
        {
        }

        void visit(const ChangesToMakeDecision &) const
        {
        }

        void visit(const NothingNoChangeDecision &) const
        {
        }
    };

    void display_one_error(
            const std::tr1::shared_ptr<Environment> &,
            const DisplayResolutionCommandLine &,
            const std::tr1::shared_ptr<const Resolution> & resolution,
            const bool verbose)
    {
        if (resolution->resolvent().slot().name_or_null())
            cout << "?   " << c::bold_red() << resolution->resolvent() << c::normal();
        else if (! resolution->resolvent().slot().null_means_unknown())
            cout << "?   " << c::bold_red() << resolution->resolvent() << c::normal();
        else
            cout << "?   " << c::bold_red() << resolution->resolvent().package()
                << " -> " << resolution->resolvent().destination_type() << c::normal();
        cout << " (no decision could be reached)" << endl;

        display_reasons(resolution, verbose);

        resolution->decision()->accept(DisplayOneErrorVisitor());
    }

    struct ChosenIDVisitor
    {
        const std::tr1::shared_ptr<const PackageID> visit(const ChangesToMakeDecision & decision) const
        {
            return decision.origin_id();
        }

        const std::tr1::shared_ptr<const PackageID> visit(const ExistingNoChangeDecision & decision) const
        {
            return decision.existing_id();
        }

        const std::tr1::shared_ptr<const PackageID> visit(const NothingNoChangeDecision &) const
        {
            return make_null_shared_ptr();
        }

        const std::tr1::shared_ptr<const PackageID> visit(const UnableToMakeDecision &) const
        {
            return make_null_shared_ptr();
        }
    };

    struct DestinationVisitor
    {
        const std::tr1::shared_ptr<const Destination> visit(const ChangesToMakeDecision & decision) const
        {
            return decision.destination();
        }

        const std::tr1::shared_ptr<const Destination> visit(const ExistingNoChangeDecision &) const
        {
            return make_null_shared_ptr();
        }

        const std::tr1::shared_ptr<const Destination> visit(const NothingNoChangeDecision &) const
        {
            return make_null_shared_ptr();
        }

        const std::tr1::shared_ptr<const Destination> visit(const UnableToMakeDecision &) const
        {
            return make_null_shared_ptr();
        }
    };

    void display_resolution_list(
            const std::tr1::shared_ptr<Environment> & env,
            const std::tr1::shared_ptr<const Resolutions> & list,
            const DisplayResolutionCommandLine & cmdline)
    {
        for (Resolutions::ConstIterator c(list->begin()), c_end(list->end()) ;
                c != c_end ; ++c)
        {
            const std::tr1::shared_ptr<const PackageID> id((*c)->decision()->accept_returning<
                    std::tr1::shared_ptr<const PackageID> >(ChosenIDVisitor()));
            if (! id)
            {
                display_one_error(env, cmdline, *c, false);
                continue;
            }

            bool is_new(false), is_upgrade(false), is_downgrade(false), is_reinstall(false),
                 other_slots(false);
            std::tr1::shared_ptr<const PackageID> old_id;

            const std::tr1::shared_ptr<const Destination> destination((*c)->decision()->accept_returning<
                    std::tr1::shared_ptr<const Destination> >(DestinationVisitor()));
            if (! destination)
                throw InternalError(PALUDIS_HERE, "huh? ! destination");

            if (destination->replacing()->empty())
            {
                is_new = true;
                const std::tr1::shared_ptr<const PackageIDSequence> others((*env)[selection::SomeArbitraryVersion(
                        generator::Package(id->name()) &
                        generator::InRepository(destination->repository())
                        )]);
                other_slots = ! others->empty();
            }
            else
                for (PackageIDSequence::ConstIterator x(destination->replacing()->begin()),
                        x_end(destination->replacing()->end()) ;
                        x != x_end ; ++x)
                {
                    old_id = *x;
                    if ((*x)->version() == id->version())
                        is_reinstall = true;
                    else if ((*x)->version() < id->version())
                        is_upgrade = true;
                    else if ((*x)->version() > id->version())
                        is_downgrade = true;
                }

            /* pick the worst of what it is */
            is_upgrade = is_upgrade && (! is_reinstall) && (! is_downgrade);
            is_reinstall = is_reinstall && (! is_downgrade);

            std::string destination_string(c::red() + "/" + c::normal());
            switch ((*c)->resolvent().destination_type())
            {
                case dt_install_to_slash:
                    destination_string = "/";
                    break;

                case dt_create_binary:
                    destination_string = "b";
                    break;

                case last_dt:
                    break;
            }

            if (! (*c)->decision()->taken())
            {
                cout << "-" << destination_string << "  " << c::blue() << id->canonical_form(idcf_no_version);
            }
            else if (is_new)
            {
                if (other_slots)
                    cout << "s" << destination_string << "  " << c::bold_blue() << id->canonical_form(idcf_no_version);
                else
                    cout << "n" << destination_string << "  " << c::bold_blue() << id->canonical_form(idcf_no_version);
            }
            else if (is_upgrade)
                cout << "u" << destination_string << "  " << c::blue() << id->canonical_form(idcf_no_version);
            else if (is_reinstall)
                cout << "r" << destination_string << "  " << c::yellow() << id->canonical_form(idcf_no_version);
            else if (is_downgrade)
                cout << "d" << destination_string << "  " << c::bold_yellow() << id->canonical_form(idcf_no_version);
            else
                throw InternalError(PALUDIS_HERE, "why did that happen?");

            cout << c::normal() << " " << id->canonical_form(idcf_version);

            cout << " to ::" << destination->repository();
            if (! destination->replacing()->empty())
            {
                cout << " replacing";
                bool first(true);
                for (PackageIDSequence::ConstIterator x(destination->replacing()->begin()),
                        x_end(destination->replacing()->end()) ;
                        x != x_end ; ++x)
                {
                    bool different(false);
                    std::string old_from;
                    if ((*x)->from_repositories_key())
                    {
                        for (Set<std::string>::ConstIterator k((*x)->from_repositories_key()->value()->begin()),
                                k_end((*x)->from_repositories_key()->value()->end()) ;
                                k != k_end ; ++k)
                        {
                            if (stringify(id->repository()->name()) != *k)
                            {
                                if (id->from_repositories_key() && (id->from_repositories_key()->value()->end() !=
                                            id->from_repositories_key()->value()->find(*k)))
                                {
                                }
                                else
                                    different = true;
                            }

                            if (old_from.empty())
                                old_from = " from ::";
                            else
                                old_from.append(", ::");

                            old_from.append(*k);
                        }
                    }

                    if (! first)
                        cout << ", ";
                    else
                        cout << " ";
                    first = false;

                    cout << (*x)->canonical_form(idcf_version);
                    if (different)
                        cout << old_from;
                }
            }

            cout << endl;

            if (id->choices_key())
            {
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
                    break;

                cout << "    " << s << endl;
            }

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

            display_reasons(*c, false);
        }

        cout << endl;
    }

    void display_resolution(
            const std::tr1::shared_ptr<Environment> & env,
            const ResolutionLists & lists,
            const DisplayResolutionCommandLine & cmdline)
    {
        Context context("When displaying chosen resolution:");

        if (lists.ordered()->empty())
        {
            if (lists.errors()->empty())
                cout << "There are no actions to carry out" << endl << endl;
            return;
        }

        cout << "These are the actions I will take, in order:" << endl << endl;
        display_resolution_list(env, lists.ordered(), cmdline);
    }

    void display_untaken(
            const std::tr1::shared_ptr<Environment> & env,
            const ResolutionLists & lists,
            const DisplayResolutionCommandLine & cmdline)
    {
        Context context("When displaying untaken resolutions:");

        if (lists.untaken()->empty())
            return;

        cout << "I didn't take the following suggestions:" << endl << endl;
        display_resolution_list(env, lists.untaken(), cmdline);
    }

    void display_errors(
            const std::tr1::shared_ptr<Environment> & env,
            const ResolutionLists & lists,
            const DisplayResolutionCommandLine & cmdline)
    {
        Context context("When displaying errors for chosen resolution:");

        if (lists.errors()->empty())
            return;

        cout << "I encountered the following errors:" << endl << endl;

        for (Resolutions::ConstIterator c(lists.errors()->begin()), c_end(lists.errors()->end()) ;
                c != c_end ; ++c)
            display_one_error(env, cmdline, *c, true);

        cout << endl;
    }

    void display_explanations(
            const std::tr1::shared_ptr<Environment> & env,
            const ResolutionLists & lists,
            const DisplayResolutionCommandLine & cmdline)
    {
        Context context("When displaying explanations:");

        if (cmdline.display_options.a_explain.begin_args() == cmdline.display_options.a_explain.end_args())
            return;

        std::cout << "Explaining requested decisions:" << std::endl << std::endl;

        for (args::StringSetArg::ConstIterator i(cmdline.display_options.a_explain.begin_args()),
                i_end(cmdline.display_options.a_explain.end_args()) ;
                i != i_end ; ++i)
        {
            bool any(false);
            PackageDepSpec spec(parse_user_package_dep_spec(*i, env.get(), UserPackageDepSpecOptions() + updso_allow_wildcards));
            for (Resolutions::ConstIterator r(lists.all()->begin()), r_end(lists.all()->end()) ;
                    r != r_end ; ++r)
            {
                const std::tr1::shared_ptr<const PackageID> id((*r)->decision()->accept_returning<
                        std::tr1::shared_ptr<const PackageID> >(ChosenIDVisitor()));
                const std::tr1::shared_ptr<const Destination> destination((*r)->decision()->accept_returning<
                        std::tr1::shared_ptr<const Destination> >(DestinationVisitor()));

                if (! id)
                {
                    /* decided nothing, so we can only work for cat/pkg, where
                     * either can be wildcards (we could work for :slot too,
                     * but we're lazy) */
                    if (! package_dep_spec_has_properties(spec, make_named_values<PackageDepSpecProperties>(
                                    value_for<n::has_additional_requirements>(false),
                                    value_for<n::has_category_name_part>(indeterminate),
                                    value_for<n::has_from_repository>(false),
                                    value_for<n::has_in_repository>(false),
                                    value_for<n::has_installable_to_path>(false),
                                    value_for<n::has_installable_to_repository>(false),
                                    value_for<n::has_installed_at_path>(false),
                                    value_for<n::has_package>(indeterminate),
                                    value_for<n::has_package_name_part>(indeterminate),
                                    value_for<n::has_slot_requirement>(false),
                                    value_for<n::has_tag>(false),
                                    value_for<n::has_version_requirements>(false)
                                    )))
                        continue;

                    if (spec.package_ptr() && *spec.package_ptr() != (*r)->resolvent().package())
                        continue;
                    if (spec.package_name_part_ptr() && *spec.package_name_part_ptr() != (*r)->resolvent().package().package())
                        continue;
                    if (spec.category_name_part_ptr() && *spec.category_name_part_ptr() != (*r)->resolvent().package().category())
                        continue;
                }
                else
                {
                    if (! match_package(*env, spec, *id, MatchPackageOptions()))
                        continue;
                }

                any = true;

                std::cout << "For " << (*r)->resolvent() << ":" << std::endl;
                std::cout << "    The following constraints were in action:" << std::endl;
                for (Constraints::ConstIterator c((*r)->constraints()->begin()),
                        c_end((*r)->constraints()->end()) ;
                        c != c_end ; ++c)
                {
                    std::cout << "      * " << (*c)->spec();

                    switch ((*c)->use_existing())
                    {
                        case ue_if_same:
                            std::cout << ", use existing if same";
                            break;
                        case ue_never:
                            std::cout << ", never using existing";
                            break;
                        case ue_only_if_transient:
                            std::cout << ", using existing only if transient";
                            break;
                        case ue_if_same_version:
                            std::cout << ", use existing if same version";
                            break;
                        case ue_if_possible:
                            std::cout << ", use existing if possible";
                            break;

                        case last_ue:
                            break;
                    }

                    switch ((*c)->destination_type())
                    {
                        case dt_install_to_slash:
                            std::cout << ", installing to /";
                            break;

                        case dt_create_binary:
                            std::cout << ", creating a binary";
                            break;

                        case last_dt:
                            break;
                    }

                    std::cout << std::endl;
                    std::cout << "        Because of ";
                    ReasonNameGetter v(true);
                    std::cout << (*c)->reason()->accept_returning<std::pair<std::string, bool> >(v).first;
                    std::cout << std::endl;
                }

                if (id)
                {
                    std::cout << "    The decision made was:" << std::endl;
                    std::cout << "        Use " << *id << std::endl;
                    if (destination)
                    {
                        std::cout << "        Install to repository " << destination->repository() << std::endl;
                        if (! destination->replacing()->empty())
                            for (PackageIDSequence::ConstIterator x(destination->replacing()->begin()),
                                    x_end(destination->replacing()->end()) ;
                                    x != x_end ; ++x)
                                std::cout << "            Replacing " << **x << std::endl;
                    }
                    std::cout << std::endl;
                }
                else
                    std::cout << "    No decision could be made" << std::endl << std::endl;
            }

            if (! any)
                throw args::DoHelp("There is nothing matching '" + *i + "' in the resolution set.");
        }
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
        const std::tr1::shared_ptr<const Sequence<std::string > > & args
        )
{
    DisplayResolutionCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_DISPLAY_RESOLUTION_OPTIONS", "CAVE_DISPLAY_RESOLUTION_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    if (getenv_with_default("PALUDIS_SERIALISED_RESOLUTION_FD", "").empty())
        throw args::DoHelp("PALUDIS_SERIALISED_RESOLUTION_FD must be provided");

    int fd(destringify<int>(getenv_with_default("PALUDIS_SERIALISED_RESOLUTION_FD", "")));
    SafeIFStream deser_stream(fd);
    const std::string deser_str((std::istreambuf_iterator<char>(deser_stream)), std::istreambuf_iterator<char>());
    Deserialiser deserialiser(env.get(), deser_str);
    Deserialisation deserialisation("ResolutionLists", deserialiser);
    ResolutionLists lists(ResolutionLists::deserialise(deserialisation));

    display_resolution(env, lists, cmdline);
    display_untaken(env, lists, cmdline);
    display_errors(env, lists, cmdline);
    display_explanations(env, lists, cmdline);

    return 0;
}

std::tr1::shared_ptr<args::ArgsHandler>
DisplayResolutionCommand::make_doc_cmdline()
{
    return make_shared_ptr(new DisplayResolutionCommandLine);
}

