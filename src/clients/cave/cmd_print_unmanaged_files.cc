/* vim: set et fdm=syntax sts=4 sw=4: */

/*
 * Copyright (c) 2012 Saleem Abdulrasool
 *
 * This file is part of the Paludis package manager.  Paludis is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License version2, as published by the Free Software Foundation.
 *
 * Paludis is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 55 Temple
 * Place, Suite 330, Boston, MA  02111-1308  USA
 */

#include "cmd_print_unmanaged_files.hh"
#include "command_command_line.hh"

#include <paludis/args/do_help.hh>
#include <paludis/args/log_level_arg.hh>

#include <paludis/filter.hh>
#include <paludis/contents.hh>
#include <paludis/generator.hh>
#include <paludis/selection.hh>
#include <paludis/package_id.hh>
#include <paludis/environment.hh>
#include <paludis/metadata_key.hh>
#include <paludis/filtered_generator.hh>

#include <paludis/util/fs_path.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/util/fs_error.hh>
#include <paludis/util/fs_iterator.hh>

#include <set>
#include <list>
#include <iostream>
#include <algorithm>

using namespace paludis;
using namespace cave;

using std::cerr;
using std::cout;
using std::endl;

namespace
{
    struct PrintUnmanagedFilesCommandLine :
        CaveCommandCommandLine
    {
        virtual std::string app_name() const
        {
            return "cave print-unmanaged-files";
        }

        virtual std::string app_synopsis() const
        {
            return "Prints a list of unmanaged files.";
        }

        virtual std::string app_description() const
        {
            return "Prints all files under a specified root which is not owned "
                   "by an installed package.  No formatting is used, making the "
                   "output suitable for parsing by scripts.";
        }

        args::ArgsGroup g_general;
        args::StringSetArg a_root;

        PrintUnmanagedFilesCommandLine() :
            g_general(main_options_section(), "General options",
                      "Options which are relevant for most or all actions"),
            a_root(&g_general, "root", 'r', "Search under the specified root")
        {
            add_usage_line("[ --root root ]");
        }
    };

    class CollectRecursiveDirectoryContents
    {
        public:
            explicit CollectRecursiveDirectoryContents(std::set<FSPath, FSPathComparator> * contents)
                : _contents(contents)
            {
            }

            ~CollectRecursiveDirectoryContents()
            {
            }

            void operator()(const FSPath& path)
            {
                for (FSIterator entry(path, { fsio_include_dotfiles, fsio_want_regular_files, fsio_want_directories }), end;
                        entry != end; ++entry)
                {
                    try
                    {
                        if (entry->stat().is_directory())
                            (*this)(*entry);
                        else
                            _contents->insert(*entry);
                    }
                    catch (const FSError& error)
                    {
                        cerr << error.message() << endl;
                    }
                }
            }

        private:
            std::set<FSPath, FSPathComparator> * const _contents;
    };

    class CollectPackageContents
    {
        public:
            CollectPackageContents(std::set<FSPath, FSPathComparator> * contents)
                : _contents(contents)
            {
            }

            ~CollectPackageContents()
            {
            }

            void operator()(const std::shared_ptr<const PackageID>& package)
            {
                const auto contents(package->contents());

                if (! contents)
                    return;

                for (auto entry(contents->begin()), end(contents->end());
                        entry != end; ++entry)
                    _contents->insert((*entry)->location_key()->parse_value());
            }

        private:
            std::set<FSPath, FSPathComparator> * const _contents;
    };
}

int
PrintUnmanagedFilesCommand::run(const std::shared_ptr<Environment> & env,
                                const std::shared_ptr< const Sequence<std::string> > & args)
{
    PrintUnmanagedFilesCommandLine cmdline;
    std::set<FSPath, FSPathComparator> roots;
    std::set<FSPath, FSPathComparator> all_files;
    std::set<FSPath, FSPathComparator> managed_files;
    std::set<FSPath, FSPathComparator> unmanaged_files;

    cmdline.run(args, "CAVE", "CAVE_PRINT_UNMANAGED_FILES_OPTIONS",
                "CAVE_PRINT_UNMANAGED_FILES_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    if (cmdline.begin_parameters() != cmdline.end_parameters())
        throw args::DoHelp("print-unmanaged-files takes no parameters");

    const auto sysroot = env->preferred_root_key()->parse_value();

    for (auto root(cmdline.a_root.begin_args()), end(cmdline.a_root.end_args());
            root != end; ++root)
    {
        const FSPath path(*root);
        const FSPath realpath(path.realpath());

        if (! realpath.stat().is_directory())
        {
            cerr << path << " is not a directory" << endl;
            return EXIT_FAILURE;
        }

        if (! realpath.starts_with(sysroot))
        {
            cerr << path << " is not under " << sysroot << endl;
            return EXIT_FAILURE;
        }

        roots.insert(realpath);
    }

    if (roots.empty())
        roots.insert(sysroot);

    std::for_each(roots.begin(), roots.end(),
                  CollectRecursiveDirectoryContents(&all_files));

    const auto selection(selection::AllVersionsUnsorted(generator::All() | filter::InstalledAtRoot(sysroot)));
    const auto packages((*env)[selection]);

    std::for_each(packages->begin(), packages->end(),
                  CollectPackageContents(&managed_files));

    std::set_difference(all_files.begin(), all_files.end(),
                        managed_files.begin(), managed_files.end(),
                        std::inserter(unmanaged_files, unmanaged_files.begin()),
                        FSPathComparator());

    for (const auto & unmanaged_file : unmanaged_files)
        cout << unmanaged_file << endl;

    return EXIT_SUCCESS;
}

std::shared_ptr<args::ArgsHandler>
PrintUnmanagedFilesCommand::make_doc_cmdline()
{
    return std::make_shared<PrintUnmanagedFilesCommandLine>();
}

CommandImportance
PrintUnmanagedFilesCommand::importance() const
{
    return ci_scripting;
}

