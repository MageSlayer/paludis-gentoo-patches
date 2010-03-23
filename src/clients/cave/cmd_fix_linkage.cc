/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Ciaran McCreesh
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

#include "cmd_fix_linkage.hh"
#include "cmd_resolve_cmdline.hh"
#include "resolve_common.hh"

#include <paludis/args/do_help.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/join.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/broken_linkage_finder.hh>
#include <paludis/package_id.hh>
#include <paludis/name.hh>
#include <paludis/dep_spec.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/metadata_key.hh>
#include <paludis/version_operator.hh>
#include <paludis/version_requirements.hh>

#include <iostream>
#include <cstdlib>

using namespace paludis;
using namespace cave;

using std::cout;
using std::endl;

bool
FixLinkageCommand::important() const
{
    return true;
}

namespace
{
    struct FixLinkageCommandLine :
        CaveCommandCommandLine
    {
        args::ArgsGroup g_linkage_options;
        args::StringArg a_library;
        args::SwitchArg a_exact;

        ResolveCommandLineResolutionOptions resolution_options;
        ResolveCommandLineExecutionOptions execution_options;
        ResolveCommandLineDisplayOptions display_options;
        ResolveCommandLineProgramOptions program_options;

        FixLinkageCommandLine() :
            g_linkage_options(main_options_section(), "Linkage options", "Options relating to linkage"),
            a_library(&g_linkage_options, "library", '\0', "Only rebuild packages linked against this library, even if it exists"),
            a_exact(&g_linkage_options, "exact", '\0', "Rebuild the same package version that is currently installed", true),
            resolution_options(this),
            execution_options(this),
            display_options(this),
            program_options(this)
        {
            add_usage_line("[ -x|--execute ] [ --library foo.so.1 ]");
        }

        std::string app_name() const
        {
            return "cave fix-linkage";
        }

        std::string app_synopsis() const
        {
            return "Identify packages with broken linkage that can be fixed by rebuilds.";
        }

        std::string app_description() const
        {
            return "Identifies packages with broken linkage that can be fixed by rebuilds. "
                "If instructed, then executes the relevant install and uninstall actions to "
                "do said fixing.";
        }
    };
}

int
FixLinkageCommand::run(
        const std::tr1::shared_ptr<Environment> & env,
        const std::tr1::shared_ptr<const Sequence<std::string > > & args
        )
{
    FixLinkageCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_FIX_LINKAGE_OPTIONS", "CAVE_FIX_LINKAGE_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    if (cmdline.begin_parameters() != cmdline.end_parameters())
        throw args::DoHelp("fix-linkage takes no parameters");

    cmdline.resolution_options.apply_shortcuts();
    cmdline.resolution_options.verify(env);

    std::string library(cmdline.a_library.argument());
    if (library.empty())
        cout << "Searching for broken packages... " << std::flush;
    else
        cout << "Searching for packages that depend on " << library << "... " << std::flush;

    BrokenLinkageFinder finder(env.get(), cmdline.a_library.argument());

    cout << endl;

    if (finder.begin_broken_packages() == finder.end_broken_packages())
    {
        if (library.empty())
            cout << "No broken packages found" << endl;
        else
            cout << "No packages that depend on " << library << " found" << endl;

        return EXIT_SUCCESS;
    }

    std::tr1::shared_ptr<Sequence<std::string> > targets(new Sequence<std::string>);

    for (BrokenLinkageFinder::BrokenPackageConstIterator pkg_it(finder.begin_broken_packages()),
             pkg_it_end(finder.end_broken_packages()); pkg_it_end != pkg_it; ++pkg_it)
    {
        cout << endl;

        cout << "* " << **pkg_it << endl;

        for (BrokenLinkageFinder::BrokenFileConstIterator file_it(finder.begin_broken_files(*pkg_it)),
                 file_it_end(finder.end_broken_files(*pkg_it)); file_it_end != file_it; ++file_it)
        {
            cout << "    " << *file_it;
            if (library.empty())
                cout << " (requires "
                          << join(finder.begin_missing_requirements(*pkg_it, *file_it),
                                  finder.end_missing_requirements(*pkg_it, *file_it),
                                  " ") << ")";
            cout << endl;
        }

        PartiallyMadePackageDepSpec part_spec((PartiallyMadePackageDepSpecOptions()));
        part_spec.package((*pkg_it)->name());
        if ((*pkg_it)->slot_key())
            part_spec.slot_requirement(make_shared_ptr(new UserSlotExactRequirement((*pkg_it)->slot_key()->value())));

        if (cmdline.a_exact.specified())
            part_spec.version_requirement(make_named_values<VersionRequirement>(
                        value_for<n::version_operator>(vo_equal),
                        value_for<n::version_spec>((*pkg_it)->version())));

        targets->push_back(stringify(PackageDepSpec(part_spec)));
    }

    std::tr1::shared_ptr<const PackageID> orphans;
    if (finder.begin_broken_files(orphans) != finder.end_broken_files(orphans))
    {
        if (library.empty())
            cout << endl << "The following broken files are not owned by any installed package:" << endl;
        else
            cout << endl << "The following files that depend on " << library << " are not owned by any installed package:" << endl;

        for (BrokenLinkageFinder::BrokenFileConstIterator file_it(finder.begin_broken_files(orphans)),
                 file_it_end(finder.end_broken_files(orphans)); file_it_end != file_it; ++file_it)
        {
            cout << "    " << *file_it;
            if (library.empty())
                cout << " (requires "
                          << join(finder.begin_missing_requirements(orphans, *file_it),
                                  finder.end_missing_requirements(orphans, *file_it),
                                  " ") << ")";
            cout << endl;
        }
    }

    return resolve_common(env, cmdline.resolution_options, cmdline.execution_options, cmdline.display_options,
            cmdline.program_options, make_null_shared_ptr(), targets);
}

std::tr1::shared_ptr<args::ArgsHandler>
FixLinkageCommand::make_doc_cmdline()
{
    return make_shared_ptr(new FixLinkageCommandLine);
}

