/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Ciaran McCreesh
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
#include <paludis/util/fs_entry.hh>
#include <paludis/util/stringify.hh>
#include <paludis/contents.hh>
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
            target = c.target();
        }

        void visit(const ContentsMiscEntry &)
        {
        }

        void visit(const ContentsDevEntry &)
        {
        }

        void visit(const ContentsFifoEntry &)
        {
        }
    };

    int number_of_parents(const FSEntry & f)
    {
        FSEntry ff(f);
        int result(0);
        while (((ff = ff.dirname())) != FSEntry("/"))
            ++result;
        return result;
    }
}

std::string
paludis::cave::format_plain_contents_entry(
        const std::tr1::shared_ptr<const ContentsEntry> & c,
        const std::string & f)
{
    ValueGetter v;
    c->accept(v);

    std::tr1::shared_ptr<Map<char, std::string> > m(new Map<char, std::string>);
    m->insert('n', c->name());
    m->insert('d', stringify(FSEntry(c->name()).dirname()));
    m->insert('b', stringify(FSEntry(c->name()).basename()));
    m->insert('t', v.target);
    m->insert('a', v.target.empty() ? "" : " -> ");
    m->insert('i', std::string(number_of_parents(FSEntry(c->name())), ' '));
    m->insert('/', v.slash);

    return format_string(f, m);
}

