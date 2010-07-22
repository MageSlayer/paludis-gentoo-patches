/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Alexander Færøy
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

#include "owner_common.hh"
#include <paludis/action.hh>
#include <paludis/contents.hh>
#include <paludis/environment.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/generator.hh>
#include <paludis/metadata_key.hh>
#include <paludis/name.hh>
#include <paludis/package_database.hh>
#include <paludis/package_id.hh>
#include <paludis/repository.hh>
#include <paludis/selection.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <algorithm>
#include <functional>

using namespace paludis;

namespace
{
    bool handle_full(const std::string & q, const std::shared_ptr<const ContentsEntry> & e)
    {
        return q == stringify(e->location_key()->value());
    }

    bool handle_basename(const std::string & q, const std::shared_ptr<const ContentsEntry> & e)
    {
        return q == e->location_key()->value().basename();
    }

    bool handle_partial(const std::string & q, const std::shared_ptr<const ContentsEntry> & e)
    {
        return std::string::npos != stringify(e->location_key()->value()).find(q);
    }
}

int
paludis::cave::owner_common(
        const std::shared_ptr<Environment> & env,
        const std::string & match,
        const std::string & query,
        const std::function<void (const std::shared_ptr<const PackageID> &)> & callback)
{
    bool found(false);
    std::function<bool (const std::string &, const std::shared_ptr<const ContentsEntry> &)> handler;

    if ("full" == match)
        handler = handle_full;
    else if ("basename" == match)
        handler = handle_basename;
    else if ("partial" == match)
        handler = handle_partial;
    else
    {
        if (! query.empty() && '/' == query.at(0))
            handler = handle_full;
        else if (std::string::npos != query.find("/"))
            handler = handle_partial;
        else
            handler = handle_basename;
    }

    std::shared_ptr<const PackageIDSequence> ids((*env)[selection::AllVersionsSorted(generator::All() |
                filter::InstalledAtRoot(env->root()))]);

    for (PackageIDSequence::ConstIterator p(ids->begin()), p_end(ids->end()); p != p_end; ++p)
    {
        if (! (*p)->contents_key())
            continue;

        std::shared_ptr<const Contents> contents((*p)->contents_key()->value());
        if (contents->end() != std::find_if(contents->begin(), contents->end(), std::bind(handler, query,
                        std::placeholders::_1)))
        {
            callback(*p);
            found = true;
        }
    }

    return found ? EXIT_SUCCESS : EXIT_FAILURE;
}

