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
#include <paludis/resolver/resolutions.hh>
#include <paludis/resolver/serialise.hh>
#include <paludis/resolver/reason.hh>
#include <paludis/resolver/sanitised_dependencies.hh>
#include <paludis/resolver/resolution.hh>
#include <paludis/resolver/decision.hh>
#include <paludis/resolver/destinations.hh>
#include <paludis/resolver/constraint.hh>
#include <paludis/resolver/resolver.hh>
#include <paludis/resolver/qpn_s.hh>
#include <paludis/package_id.hh>
#include <paludis/version_spec.hh>
#include <paludis/metadata_key.hh>
#include <paludis/choice.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/match_package.hh>

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
        std::pair<std::string, bool> visit(const DependencyReason & r) const
        {
            if (r.sanitised_dependency().spec().if_block())
                return std::make_pair(stringify(r.from_id()->name()) + " blocker " +
                            stringify(*r.sanitised_dependency().spec().if_block()), true);
            else
                return std::make_pair(stringify(r.from_id()->name()), false);
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

    void display_resolution(
            const std::tr1::shared_ptr<Environment> &,
            const ResolutionLists & lists,
            const DisplayResolutionCommandLine &)
    {
        Context context("When displaying chosen resolution:");

        cout << "These are the actions I will take, in order:" << endl << endl;

        for (Resolutions::ConstIterator c(lists.ordered()->begin()), c_end(lists.ordered()->end()) ;
                c != c_end ; ++c)
        {
            const std::tr1::shared_ptr<const PackageID> id((*c)->decision()->if_package_id());
            if (! id)
                throw InternalError(PALUDIS_HERE, "why did that happen?");

            bool is_new(false), is_upgrade(false), is_downgrade(false), is_reinstall(false);
            std::tr1::shared_ptr<const PackageID> old_id;

            if ((*c)->destinations()->slash())
            {
                if ((*c)->destinations()->slash()->replacing()->empty())
                    is_new = true;
                else
                    for (PackageIDSequence::ConstIterator x((*c)->destinations()->slash()->replacing()->begin()),
                            x_end((*c)->destinations()->slash()->replacing()->end()) ;
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
            }

            if (is_new)
                cout << "[n] " << c::bold_blue() << id->canonical_form(idcf_no_version);
            else if (is_upgrade)
                cout << "[u] " << c::blue() << id->canonical_form(idcf_no_version);
            else if (is_reinstall)
                cout << "[r] " << c::yellow() << id->canonical_form(idcf_no_version);
            else if (is_downgrade)
                cout << "[d] " << c::bold_yellow() << id->canonical_form(idcf_no_version);
            else
                throw InternalError(PALUDIS_HERE, "why did that happen?");

            cout << c::normal() << " " << id->canonical_form(idcf_version);

            if ((*c)->destinations()->slash())
            {
                cout << " to ::" << (*c)->destinations()->slash()->repository();
                if (! (*c)->destinations()->slash()->replacing()->empty())
                {
                    cout << " replacing";
                    for (PackageIDSequence::ConstIterator x((*c)->destinations()->slash()->replacing()->begin()),
                            x_end((*c)->destinations()->slash()->replacing()->end()) ;
                            x != x_end ; ++x)
                        cout << " " << (*x)->canonical_form(idcf_version);
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
                cout << "    \"" << id->short_description_key()->value() << "\"" << endl;

            std::set<std::string> reason_names, special_reason_names;
            for (Constraints::ConstIterator r((*c)->constraints()->begin()),
                    r_end((*c)->constraints()->end()) ;
                    r != r_end ; ++r)
            {
                ReasonNameGetter g;
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

                if ((! (*c)->decision()->is_best()) && (! (*c)->decision()->is_nothing())
                        && (! (*c)->decision()->is_installed()))
                    cout << c::bold_red() << " which prevented selection of the best candidate" << c::normal();

                cout << endl;
            }
        }

        cout << endl;
    }

    struct ReasonDisplayer
    {
        void visit(const TargetReason &)
        {
            std::cout << "it is a target";
        }

        void visit(const DependencyReason & reason)
        {
            std::cout << "of dependency " << reason.sanitised_dependency() << " from " << *reason.from_id();
        }

        void visit(const PresetReason &)
        {
            std::cout << "of a preset";
        }

        void visit(const SetReason & reason)
        {
            std::cout << "of set " << reason.set_name() << ", which is because ";
            reason.reason_for_set()->accept(*this);
        }
    };

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
                if (! (*r)->decision()->if_package_id())
                {
                    /* really we want this to work for simple cat/pkg and
                     * cat/pkg:slot specs anyway, even if we chose nothing */
                    continue;
                }

                if (! match_package(*env, spec, *(*r)->decision()->if_package_id(), MatchPackageOptions()))
                    continue;

                any = true;

                std::cout << "For " << (*r)->qpn_s() << ":" << std::endl;
                std::cout << "    The following constraints were in action:" << std::endl;
                for (Constraints::ConstIterator c((*r)->constraints()->begin()),
                        c_end((*r)->constraints()->end()) ;
                        c != c_end ; ++c)
                {
                    std::cout << "      * " << (*c)->spec();

                    switch ((*c)->use_installed())
                    {
                        case ui_if_same:
                            std::cout << ", use installed if same";
                            break;
                        case ui_never:
                            std::cout << ", never using installed";
                            break;
                        case ui_only_if_transient:
                            std::cout << ", using installed only if transient";
                            break;
                        case ui_if_same_version:
                            std::cout << ", use installed if same version";
                            break;
                        case ui_if_possible:
                            std::cout << ", use installed if possible";
                            break;

                        case last_ui:
                            break;
                    }

                    if ((*c)->to_destination_slash())
                        std::cout << ", installing to /";

                    std::cout << std::endl;
                    std::cout << "        because ";
                    ReasonDisplayer v;
                    (*c)->reason()->accept(v);
                    std::cout << std::endl;
                }
                std::cout << "    The decision made was:" << std::endl;
                std::cout << "        Use " << *(*r)->decision()->if_package_id() << std::endl;
                if ((*r)->destinations()->slash())
                {
                    std::cout << "        Install to / using repository " << (*r)->destinations()->slash()->repository() << std::endl;
                    if (! (*r)->destinations()->slash()->replacing()->empty())
                        for (PackageIDSequence::ConstIterator x((*r)->destinations()->slash()->replacing()->begin()),
                                x_end((*r)->destinations()->slash()->replacing()->end()) ;
                                x != x_end ; ++x)
                            std::cout << "            Replacing " << **x << std::endl;
                }
                std::cout << std::endl;
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
    display_explanations(env, lists, cmdline);

    return 0;
}

std::tr1::shared_ptr<args::ArgsHandler>
DisplayResolutionCommand::make_doc_cmdline()
{
    return make_shared_ptr(new DisplayResolutionCommandLine);
}

