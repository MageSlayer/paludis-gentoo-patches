/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#include "cmd_print_categories.hh"
#include <paludis/args/args.hh>
#include <paludis/args/do_help.hh>
#include <paludis/name.hh>
#include <paludis/environment.hh>
#include <paludis/repository.hh>
#include <paludis/util/set.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/stringify.hh>
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
    struct PrintCategoriesCommandLine :
        CaveCommandCommandLine
    {
        virtual std::string app_name() const
        {
            return "cave print-categories";
        }

        virtual std::string app_synopsis() const
        {
            return "Prints a list of known categories.";
        }

        virtual std::string app_description() const
        {
            return "Prints a list of known categories. No formatting is used, making the output suitable for "
                "parsing by scripts.";
        }

        args::ArgsGroup g_filters;
        args::StringSetArg a_containing;
        args::StringSetArg a_repository;

        PrintCategoriesCommandLine() :
            g_filters(main_options_section(), "Filters", "Filter the output. Each filter may be specified more than once. The object "
                    "specified by the filter does not have to exist."),
            a_containing(&g_filters, "containing", '\0', "Show only categories containing this package name. If specified "
                    "multiple times, categories containing any of these package names are selected."),
            a_repository(&g_filters, "repository", '\0', "Show only categories in this repository. If specified multiple "
                    "times, categories in any of these repositories are selected.")
        {
            add_usage_line("[ --containing pkgname ] [ --repository reponame ]");
        }
    };
}

int
PrintCategoriesCommand::run(
        const std::shared_ptr<Environment> & env,
        const std::shared_ptr<const Sequence<std::string > > & args
        )
{
    PrintCategoriesCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_PRINT_CATEGORIES_OPTIONS", "CAVE_PRINT_CATEGORIES_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    if (cmdline.begin_parameters() != cmdline.end_parameters())
        throw args::DoHelp("print-categories takes no parameters");

    std::set<CategoryNamePart> categories;
    for (auto r(env->begin_repositories()), r_end(env->end_repositories()) ;
            r != r_end ; ++r)
    {
        if (cmdline.a_repository.specified())
            if (cmdline.a_repository.end_args() == std::find(cmdline.a_repository.begin_args(),
                        cmdline.a_repository.end_args(), stringify((*r)->name())))
                continue;

        if (cmdline.a_containing.specified())
        {
            for (args::StringSetArg::ConstIterator p(cmdline.a_containing.begin_args()), p_end(cmdline.a_containing.end_args()) ;
                    p != p_end ; ++p)
            {
                std::shared_ptr<const CategoryNamePartSet> cats((*r)->category_names_containing_package(PackageNamePart(*p), { }));
                std::copy(cats->begin(), cats->end(), std::inserter(categories, categories.begin()));
            }
        }
        else
        {
            std::shared_ptr<const CategoryNamePartSet> cats((*r)->category_names({ }));
            std::copy(cats->begin(), cats->end(), std::inserter(categories, categories.begin()));
        }
    }

    std::copy(categories.begin(), categories.end(), std::ostream_iterator<CategoryNamePart>(cout, "\n"));

    return EXIT_SUCCESS;
}

std::shared_ptr<args::ArgsHandler>
PrintCategoriesCommand::make_doc_cmdline()
{
    return std::make_shared<PrintCategoriesCommandLine>();
}

CommandImportance
PrintCategoriesCommand::importance() const
{
    return ci_scripting;
}

