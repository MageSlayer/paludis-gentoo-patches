/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009 Alexander Færøy
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

#include "executables_common.hh"
#include "exceptions.hh"
#include "parse_spec_with_nice_error.hh"
#include <paludis/contents.hh>
#include <paludis/environment.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/generator.hh>
#include <paludis/metadata_key.hh>
#include <paludis/package_id.hh>
#include <paludis/selection.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/util/fs_path.hh>
#include <paludis/util/fs_error.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/system.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/accept_visitor.hh>
#include <paludis/util/make_null_shared_ptr.hh>

#include <algorithm>
#include <set>
#include <cstdlib>
#include <memory>

using namespace paludis;
using namespace cave;

namespace
{
    class ExecutablesDisplayer
    {
        private:
            const std::set<std::string> & _paths;
            const std::function<void (const FSPath &)> _displayer;

            bool is_executable_in_path(const FSPath & file)
            {
                try
                {
                    FSStat file_stat(file);

                    return file_stat.exists() && (0 != (file_stat.permissions() & (S_IXUSR | S_IXGRP | S_IXOTH))) &&
                        _paths.end() != _paths.find(stringify(file.dirname()));
                }
                catch (const FSError &)
                {
                    return false;
                }
            }

        public:
            ExecutablesDisplayer(
                    const std::set<std::string> & p,
                    const std::function<void (const FSPath &)> & d) :
                _paths(p),
                _displayer(d)
            {
            }

            void visit(const ContentsFileEntry & e)
            {
                if (is_executable_in_path(e.location_key()->parse_value()))
                    _displayer(e.location_key()->parse_value());
            }

            void visit(const ContentsSymEntry & e)
            {
                FSPath symlink(e.location_key()->parse_value());
                FSPath real_file(symlink.realpath_if_exists());

                if (symlink != real_file && is_executable_in_path(symlink))
                    _displayer(e.location_key()->parse_value());
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
        const std::shared_ptr<Environment> & env,
        const std::string & param,
        const std::function<void (const FSPath &)> & displayer,
        const bool all,
        const bool best)
{
    PackageDepSpec spec(parse_spec_with_nice_error(param, env.get(), { updso_allow_wildcards },
                filter::InstalledAtRoot(env->preferred_root_key()->parse_value())));

    std::shared_ptr<const PackageIDSequence> entries(
            (*env)[selection::AllVersionsSorted(generator::Matches(spec, make_null_shared_ptr(), { }) |
                filter::InstalledAtRoot(env->preferred_root_key()->parse_value()))]);

    if (entries->empty())
        throw NoSuchPackageError(param);

    if ((! best) && (! all) && (next(entries->begin()) != entries->end()))
        throw BeMoreSpecific(spec, entries);

    const std::string path(getenv_or_error("PATH"));
    std::set<std::string> paths;
    tokenise<delim_kind::AnyOfTag, delim_mode::DelimiterTag>(path, ":", "", std::inserter(paths, paths.begin()));
    ExecutablesDisplayer ed(paths, displayer);

    for (auto i(best ? entries->last() : entries->begin()), i_end(entries->end()) ;
            i != i_end ; ++i)
    {
        if ((*i)->contents_key())
        {
            std::shared_ptr<const Contents> contents((*i)->contents_key()->parse_value());
            std::for_each(indirect_iterator(contents->begin()), indirect_iterator(contents->end()), accept_visitor(ed));
        }
    }

    return EXIT_SUCCESS;
}

