/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2011 Ciaran McCreesh
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

#include "cmd_report.hh"
#include "format_user_config.hh"
#include "colours.hh"
#include "exceptions.hh"

#include <paludis/args/args.hh>
#include <paludis/args/do_help.hh>

#include <paludis/name.hh>
#include <paludis/environment.hh>
#include <paludis/repository.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/filter.hh>
#include <paludis/generator.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/selection.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/mask.hh>
#include <paludis/match_package.hh>
#include <paludis/partially_made_package_dep_spec.hh>
#include <paludis/version_requirements.hh>
#include <paludis/action.hh>

#include <paludis/util/set.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/util/accept_visitor.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/join.hh>

#include <paludis/resolver/collect_purges.hh>
#include <paludis/resolver/collect_installed.hh>

#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <set>

#include "command_command_line.hh"

using namespace paludis;
using namespace cave;
using std::cout;
using std::endl;

namespace
{
#include "cmd_report-fmt.hh"

    struct ReportCommandLine :
        CaveCommandCommandLine
    {
        virtual std::string app_name() const
        {
            return "cave report";
        }

        virtual std::string app_synopsis() const
        {
            return "Display a summary of potential problems with installed packages.";
        }

        virtual std::string app_description() const
        {
            return "Displays a formatted summary of potential problems with installed packages.";
        }

        ReportCommandLine()
        {
            add_usage_line("");
        }
    };

    const std::shared_ptr<const PackageID> find_origin_for(
            const std::shared_ptr<Environment> & env,
            const std::shared_ptr<const PackageID> & id)
    {
        if (id->from_repositories_key())
        {
            for (auto r(id->from_repositories_key()->value()->begin()), r_end(id->from_repositories_key()->value()->end()) ;
                    r != r_end ; ++r)
            {
                auto ids((*env)[selection::BestVersionOnly((
                                generator::InRepository(RepositoryName(*r)) &
                                generator::Matches(make_package_dep_spec({ })
                                    .package(id->name())
                                    .version_requirement(make_named_values<VersionRequirement>(
                                            n::version_operator() = vo_equal,
                                            n::version_spec() = id->version())),
                                    make_null_shared_ptr(), { })) |
                            filter::SupportsAction<InstallAction>())]);

                if (! ids->empty())
                    return *ids->begin();
            }
        }

        return make_null_shared_ptr();
    }

    void need_heading(
            bool & already,
            const std::shared_ptr<const PackageID> & id)
    {
        if (! already)
        {
            cout << fuc(fs_package_id(), fv<'s'>(stringify(*id)));
            already = true;
        }
    }

    void need_heading_origin(
            bool & already,
            bool & already_origin,
            const std::shared_ptr<const PackageID> & id,
            const std::shared_ptr<const PackageID> & origin_id)
    {
        need_heading(already, id);

        if (! already_origin)
        {
            cout << fuc(fs_package_origin(), fv<'s'>(stringify(*origin_id)));
            already_origin = true;
        }
    }

    void do_nothing()
    {
    }
}

int
ReportCommand::run(
        const std::shared_ptr<Environment> & env,
        const std::shared_ptr<const Sequence<std::string > > & args
        )
{
    ReportCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_REPORT_OPTIONS", "CAVE_REPORT_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    if (cmdline.begin_parameters() != cmdline.end_parameters())
        throw args::DoHelp("report takes no parameters");

    auto ids((*env)[selection::AllVersionsSorted(
                generator::All() | filter::InstalledAtRoot(env->preferred_root_key()->value()))]);

    auto insecurity(env->set(SetName("insecurity")));

    auto have_now(resolver::collect_installed(env.get()));
    auto have_now_seq(std::make_shared<PackageIDSequence>());
    std::copy(have_now->begin(), have_now->end(), have_now_seq->back_inserter());
    auto unused(resolver::collect_purges(env.get(), have_now, have_now_seq, do_nothing));

    bool errors(false);

    for (auto i(ids->begin()), i_end(ids->end()) ;
            i != i_end ; ++i)
    {
        if ((*i)->virtual_for_key())
        {
            /* too weird */
            continue;
        }

        bool done_heading(false), done_heading_origin(false);

        auto origin(find_origin_for(env, *i));
        if (! origin)
        {
            if ((*i)->behaviours_key() &&
                    (*i)->behaviours_key()->value()->end() != (*i)->behaviours_key()->value()->find("transient"))
            {
                /* that's ok */
            }
            else
            {
                need_heading(done_heading, *i);
                std::string repos;
                if ((*i)->from_repositories_key())
                    repos = join((*i)->from_repositories_key()->value()->begin(), (*i)->from_repositories_key()->value()->end(), ", ");
                cout << fuc(fs_package_no_origin(), fv<'s'>(repos));
            }
        }
        else
        {
            if (origin->masked())
            {
                need_heading_origin(done_heading, done_heading_origin, *i, origin);
                cout << fuc(fs_package_origin_masked());
            }
        }

        if (insecurity && match_package_in_set(*env, *insecurity, *i, { }))
        {
            need_heading(done_heading, *i);
            cout << fuc(fs_package_insecure());
        }

        if (unused->end() != unused->find(*i))
        {
            if (((*i)->behaviours_key() && (*i)->behaviours_key()->value()->end() !=
                        (*i)->behaviours_key()->value()->find("used")) ||
                    (! (*i)->supports_action(SupportsActionTest<UninstallAction>())))
            {
                /* ok, or weird */
            }
            else
            {
                need_heading(done_heading, *i);
                cout << fuc(fs_package_unused());
            }
        }

        if (done_heading)
            errors = true;
    }

    return errors ? EXIT_FAILURE : EXIT_SUCCESS;
}

std::shared_ptr<args::ArgsHandler>
ReportCommand::make_doc_cmdline()
{
    return std::make_shared<ReportCommandLine>();
}

CommandImportance
ReportCommand::importance() const
{
    return ci_core;
}

