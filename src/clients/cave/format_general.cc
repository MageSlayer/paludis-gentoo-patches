/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2010 Ciaran McCreesh
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

#include "format_general.hh"
#include "format_string.hh"
#include <paludis/util/map.hh>
#include <paludis/util/stringify.hh>

using namespace paludis;
using namespace cave;

std::string
paludis::cave::format_general_s(const std::string & f, const std::string & s)
{
    std::tr1::shared_ptr<Map<char, std::string> > m(new Map<char, std::string>);
    m->insert('s', s);
    return format_string(f, m);
}

std::string
paludis::cave::format_general_si(const std::string & f, const std::string & s, const int i)
{
    std::tr1::shared_ptr<Map<char, std::string> > m(new Map<char, std::string>);
    m->insert('s', s);
    m->insert('i', std::string(i, ' '));
    return format_string(f, m);
}

std::string
paludis::cave::format_general_kv(const std::string & f, const std::string & k, const std::string & v)
{
    std::tr1::shared_ptr<Map<char, std::string> > m(new Map<char, std::string>);
    m->insert('k', k);
    m->insert('v', v);
    return format_string(f, m);
}

std::string
paludis::cave::format_general_sr(const std::string & f, const std::string & s, const std::string & r)
{
    std::tr1::shared_ptr<Map<char, std::string> > m(new Map<char, std::string>);
    m->insert('s', s);
    m->insert('r', r);
    return format_string(f, m);
}

std::string
paludis::cave::format_general_spad(const std::string & f, const std::string & s, const int p, const int a, const int d)
{
    std::tr1::shared_ptr<Map<char, std::string> > m(new Map<char, std::string>);
    m->insert('s', s);
    m->insert('p', stringify(p));
    m->insert('a', stringify(a));
    m->insert('d', stringify(d));
    return format_string(f, m);
}

std::string
paludis::cave::format_general_rhvib(const std::string & f, const std::string & r, const std::string & h,
        const std::string & v, const int i, const bool b)
{
    std::tr1::shared_ptr<Map<char, std::string> > m(new Map<char, std::string>);
    m->insert('r', r);
    m->insert('h', h);
    m->insert('v', v);
    m->insert('i', std::string(i, ' '));
    m->insert('b', b ? "true" : "");
    return format_string(f, m);
}

std::string
paludis::cave::format_general_i(const std::string & f, const int i)
{
    std::tr1::shared_ptr<Map<char, std::string> > m(new Map<char, std::string>);
    m->insert('i', std::string(i, ' '));
    return format_string(f, m);
}

std::string
paludis::cave::format_general_his(const std::string & f, const std::string & h, const int i, const std::string & s)
{
    std::tr1::shared_ptr<Map<char, std::string> > m(new Map<char, std::string>);
    m->insert('h', h);
    m->insert('i', std::string(i, ' '));
    m->insert('s', s);
    return format_string(f, m);
}

