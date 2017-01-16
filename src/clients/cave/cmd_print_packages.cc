/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Alexander Færøy
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

#include "cmd_print_packages.hh"
#include "command_command_line.hh"

#include <paludis/action.hh>
#include <paludis/args/args.hh>
#include <paludis/args/do_help.hh>
#include <paludis/environment.hh>
#include <paludis/repository.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/set.hh>
#include <paludis/util/stringify.hh>

#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <set>

using namespace paludis;
using namespace cave;
using std::cout;
using std::endl;

namespace
{
    struct PrintPackagesCommandLine :
        CaveCommandCommandLine
    {
        std::string app_name() const override
        {
            return "cave print-packages";
        }

        std::string app_synopsis() const override
        {
            return "Prints a list of package names.";
        }

        std::string app_description() const override
        {
            return "Prints a list of package names. No formatting is used, making the output suitable for parsing by scripts.";
        }

        args::ArgsGroup g_filters;
        args::StringSetArg a_repository;
        args::StringSetArg a_category;

        PrintPackagesCommandLine() :
            g_filters(main_options_section(), "Filters", "Filter the output. Each filter may be specified more than once."),
            a_repository(&g_filters, "repository", 'r', "Show only names in the specified repository.",
                    args::StringSetArg::StringSetArgOptions()),
            a_category(&g_filters, "category", 'c', "Show only names in the specified category.",
                    args::StringSetArg::StringSetArgOptions())
        {
        }
    };
}

int
PrintPackagesCommand::run(
        const std::shared_ptr<Environment> & env,
        const std::shared_ptr<const Sequence<std::string > > & args
        )
{
    PrintPackagesCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_PRINT_PACKAGES_OPTIONS", "CAVE_PRINT_PACKAGES_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    if (cmdline.begin_parameters() != cmdline.end_parameters())
        throw args::DoHelp("print-packages takes no parameters");

    std::set<QualifiedPackageName> all_packages;

    for (const auto & repository : env->repositories())
    {
        if (cmdline.a_repository.specified())
            if (cmdline.a_repository.end_args() == std::find(cmdline.a_repository.begin_args(), cmdline.a_repository.end_args(), stringify(repository->name())))
                continue;

        std::shared_ptr<const CategoryNamePartSet> categories(repository->category_names({}));
        for (CategoryNamePartSet::ConstIterator c(categories->begin()), c_end(categories->end());
                c != c_end; ++c)
        {
            if (cmdline.a_category.specified())
                if (cmdline.a_category.end_args() == std::find(
                            cmdline.a_category.begin_args(), cmdline.a_category.end_args(), stringify(*c)))
                    continue;

            std::shared_ptr<const QualifiedPackageNameSet> packages(repository->package_names(*c, { }));
            std::copy(packages->begin(), packages->end(), std::inserter(all_packages, all_packages.begin()));
        }
    }

    std::copy(all_packages.begin(), all_packages.end(), std::ostream_iterator<QualifiedPackageName>(cout, "\n"));

    return EXIT_SUCCESS;
}

std::shared_ptr<args::ArgsHandler>
PrintPackagesCommand::make_doc_cmdline()
{
    return std::make_shared<PrintPackagesCommandLine>();
}
CommandImportance
PrintPackagesCommand::importance() const
{
    return ci_scripting;
}

