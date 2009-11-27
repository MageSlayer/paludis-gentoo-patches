/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009 Alexander Færøy
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

#include "cmd_print_id_executables.hh"
#include "command_command_line.hh"

#include <paludis/args/args.hh>
#include <paludis/args/do_help.hh>
#include <paludis/contents.hh>
#include <paludis/environment.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/generator.hh>
#include <paludis/metadata_key.hh>
#include <paludis/package_database.hh>
#include <paludis/package_id.hh>
#include <paludis/selection.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/system.hh>
#include <paludis/util/tokeniser.hh>

#include <iostream>
#include <algorithm>
#include <set>
#include <cstdlib>
#include <tr1/memory>

using namespace paludis;
using namespace cave;
using std::cout;
using std::endl;

namespace
{
    struct PrintIDExecutablesCommandLine :
        CaveCommandCommandLine
    {
        virtual std::string app_name() const
        {
            return "cave print-id-executables";
        }

        virtual std::string app_synopsis() const
        {
            return "Prints a list of executables belonging to an ID.";
        }

        virtual std::string app_description() const
        {
            return "Prints a list of executables belonging to an ID. "
                "No formating is used, making the script suitable for parsing by scripts.";
        }

        PrintIDExecutablesCommandLine()
        {
            add_usage_line("spec");
        }
    };

    class ExecutablesDisplayer
    {
        private:
            const std::set<std::string> & _paths;

            bool is_executable_in_path(const FSEntry & file)
            {
                try
                {
                    return file.exists() &&
                        (file.has_permission(fs_ug_owner, fs_perm_execute) ||
                        file.has_permission(fs_ug_group, fs_perm_execute) ||
                        file.has_permission(fs_ug_others, fs_perm_execute)) &&
                        _paths.end() != _paths.find(stringify(file.dirname()));
                }
                catch (const paludis::FSError &)
                {
                    return false;
                }
            }

        public:
            ExecutablesDisplayer(const std::set<std::string> & p) :
                _paths(p)
            {
            }

            void visit(const ContentsFileEntry & e)
            {
                if (is_executable_in_path(e.location_key()->value()))
                    cout << e.location_key()->value() << endl;
            }

            void visit(const ContentsSymEntry & e)
            {
                FSEntry symlink(e.location_key()->value());
                FSEntry real_file(symlink.realpath_if_exists());

                if (symlink != real_file && is_executable_in_path(symlink))
                    cout << e.location_key()->value() << endl;
            }

            void visit(const ContentsDirEntry &)
            {
            }

            void visit(const ContentsOtherEntry &)
            {
            }
    };
}

int
PrintIDExecutablesCommand::run(
        const std::tr1::shared_ptr<Environment> & env,
        const std::tr1::shared_ptr<const Sequence<std::string > > & args
        )
{
    PrintIDExecutablesCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_PRINT_ID_EXECUTABLES_OPTIONS", "CAVE_PRINT_ID_EXECUTABLES_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    if (1 != std::distance(cmdline.begin_parameters(), cmdline.end_parameters()))
        throw args::DoHelp("print-id-executables takes exactly one parameter");

    std::tr1::shared_ptr<const PackageIDSequence> entries(
            (*env)[selection::AllVersionsSorted(generator::Matches(
                    PackageDepSpec(parse_user_package_dep_spec(
                            *cmdline.begin_parameters(), env.get(),
                            UserPackageDepSpecOptions() + updso_allow_wildcards,
                            filter::InstalledAtRoot(env->root()))),
                    MatchPackageOptions()) | filter::InstalledAtRoot(env->root()))]);

    if (entries->empty())
        throw NoSuchPackageError(*cmdline.begin_parameters());

    const std::string path(getenv_or_error("PATH"));
    std::set<std::string> paths;
    tokenise<delim_kind::AnyOfTag, delim_mode::DelimiterTag>(path, ":", "", std::inserter(paths, paths.begin()));
    ExecutablesDisplayer ed(paths);

    for (PackageIDSequence::ConstIterator t(entries->begin()), t_end(entries->end()) ; t != t_end ; ++t)
    {
        if ((*t)->contents_key())
        {
            std::tr1::shared_ptr<const Contents> contents((*t)->contents_key()->value());
            std::for_each(indirect_iterator(contents->begin()), indirect_iterator(contents->end()), accept_visitor(ed));
        }
    }

    return EXIT_SUCCESS;
}

std::tr1::shared_ptr<args::ArgsHandler>
PrintIDExecutablesCommand::make_doc_cmdline()
{
    return make_shared_ptr(new PrintIDExecutablesCommandLine);
}
