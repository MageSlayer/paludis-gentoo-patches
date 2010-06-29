/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009 Alexander Færøy
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

#include "executables_common.hh"
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

#include <algorithm>
#include <set>
#include <cstdlib>
#include <tr1/memory>

using namespace paludis;
using namespace cave;

namespace
{
    class ExecutablesDisplayer
    {
        private:
            const std::set<std::string> & _paths;
            const std::tr1::function<void (const FSEntry &)> _displayer;

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
            ExecutablesDisplayer(
                    const std::set<std::string> & p,
                    const std::tr1::function<void (const FSEntry &)> & d) :
                _paths(p),
                _displayer(d)
            {
            }

            void visit(const ContentsFileEntry & e)
            {
                if (is_executable_in_path(e.location_key()->value()))
                    _displayer(e.location_key()->value());
            }

            void visit(const ContentsSymEntry & e)
            {
                FSEntry symlink(e.location_key()->value());
                FSEntry real_file(symlink.realpath_if_exists());

                if (symlink != real_file && is_executable_in_path(symlink))
                    _displayer(e.location_key()->value());
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
paludis::cave::executables_common(
        const std::tr1::shared_ptr<Environment> & env,
        const std::string & param,
        const std::tr1::function<void (const FSEntry &)> & displayer)
{
    std::tr1::shared_ptr<const PackageIDSequence> entries(
            (*env)[selection::AllVersionsSorted(generator::Matches(
                    PackageDepSpec(parse_user_package_dep_spec(
                            param, env.get(),
                            UserPackageDepSpecOptions() + updso_allow_wildcards,
                            filter::InstalledAtRoot(env->root()))),
                    MatchPackageOptions()) | filter::InstalledAtRoot(env->root()))]);

    if (entries->empty())
        throw NoSuchPackageError(param);

    const std::string path(getenv_or_error("PATH"));
    std::set<std::string> paths;
    tokenise<delim_kind::AnyOfTag, delim_mode::DelimiterTag>(path, ":", "", std::inserter(paths, paths.begin()));
    ExecutablesDisplayer ed(paths, displayer);

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

