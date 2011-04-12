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

#include "format_plain_contents_entry.hh"
#include "format_string.hh"
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/map.hh>
#include <paludis/util/fs_path.hh>
#include <paludis/util/stringify.hh>
#include <paludis/contents.hh>
#include <paludis/metadata_key.hh>
#include <sstream>

using namespace paludis;
using namespace cave;

namespace
{
    struct ValueGetter
    {
        std::string target;
        std::string slash;

        void visit(const ContentsFileEntry &)
        {
        }

        void visit(const ContentsDirEntry &)
        {
            slash = "/";
        }

        void visit(const ContentsSymEntry & c)
        {
            target = c.target_key()->parse_value();
        }

        void visit(const ContentsOtherEntry &)
        {
        }
    };

    int number_of_parents(const FSPath & f)
    {
        FSPath ff(f);
        int result(0);
        while (((ff = ff.dirname())) != FSPath("/"))
            ++result;
        return result;
    }
}

std::string
paludis::cave::format_plain_contents_entry(
        const std::shared_ptr<const ContentsEntry> & c,
        const std::string & f)
{
    ValueGetter v;
    c->accept(v);

    std::shared_ptr<Map<char, std::string> > m(std::make_shared<Map<char, std::string>>());
    m->insert('n', stringify(c->location_key()->parse_value()));
    m->insert('d', stringify(c->location_key()->parse_value().dirname()));
    m->insert('b', stringify(c->location_key()->parse_value().basename()));
    m->insert('t', v.target);
    m->insert('a', v.target.empty() ? "" : " -> ");
    m->insert('i', std::string(number_of_parents(c->location_key()->parse_value()), ' '));
    m->insert('/', v.slash);

    return format_string(f, m);
}

