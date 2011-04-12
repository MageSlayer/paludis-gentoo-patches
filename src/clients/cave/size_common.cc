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

#include "size_common.hh"
#include "exceptions.hh"
#include "parse_spec_with_nice_error.hh"
#include <paludis/action.hh>
#include <paludis/contents.hh>
#include <paludis/environment.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/generator.hh>
#include <paludis/metadata_key.hh>
#include <paludis/name.hh>
#include <paludis/package_id.hh>
#include <paludis/repository.hh>
#include <paludis/selection.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/pretty_print.hh>
#include <paludis/util/log.hh>
#include <paludis/util/fs_path.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <algorithm>
#include <functional>
#include <iostream>

using namespace paludis;

using std::cout;
using std::endl;

namespace
{
    struct GetSize
    {
        unsigned long visit(const ContentsDirEntry &) const
        {
            return 0;
        }

        unsigned long visit(const ContentsSymEntry &) const
        {
            return 0;
        }

        unsigned long visit(const ContentsOtherEntry &) const
        {
            return 0;
        }

        unsigned long visit(const ContentsFileEntry & e) const
        {
            FSPath path(e.location_key()->parse_value());
            FSStat stat(path);

            if (stat.is_regular_file_or_symlink_to_regular_file())
                return stat.file_size();
            else
            {
                Log::get_instance()->message("cave.size.missing", ll_warning, lc_context) << "Couldn't get size for '"
                    << path << "'";
                return 0;
            }
        }
    };
}

int
paludis::cave::size_common(
        const std::shared_ptr<Environment> & env,
        const bool purdy,
        const std::string & q,
        const bool all,
        const bool best)
{
    PackageDepSpec spec(parse_spec_with_nice_error(q, env.get(), { }, filter::All()));
    std::shared_ptr<const PackageIDSequence> entries((*env)[selection::AllVersionsSorted(generator::Matches(spec, make_null_shared_ptr(), { }) |
                filter::InstalledAtRoot(env->preferred_root_key()->parse_value()))]);

    if (entries->empty())
        return EXIT_FAILURE;
    else if ((! best) && (! all) && (next(entries->begin()) != entries->end()))
        throw BeMoreSpecific(spec, entries);

    for (auto i(best ? entries->last() : entries->begin()), i_end(entries->end()) ;
            i != i_end ; ++i)
    {
        if (! (*i)->contents_key())
            throw BadIDForCommand(spec, (*i), "does not support listing contents");

        unsigned long size(0);
        auto contents((*i)->contents_key()->parse_value());
        for (auto c(contents->begin()), c_end(contents->end()) ;
                c != c_end ; ++c)
            size += (*c)->accept_returning<unsigned long>(GetSize());

        if (purdy)
            cout << pretty_print_bytes(size) << endl;
        else
            cout << size << endl;
    }

    return EXIT_SUCCESS;
}

