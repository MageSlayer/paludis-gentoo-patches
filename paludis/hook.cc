/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include "hook.hh"
#include <map>

using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<Hook>
    {
        std::string name;
        std::map<std::string, std::string> extra_env;

        Implementation(const std::string & n, const std::map<std::string, std::string> & e) :
            name(n),
            extra_env(e)
        {
        }
    };
}

Hook::Hook(const std::string & n) :
    PrivateImplementationPattern<Hook>(new Implementation<Hook>(n, std::map<std::string, std::string>()))
{
}

Hook::Hook(const Hook & h) :
    PrivateImplementationPattern<Hook>(new Implementation<Hook>(h._imp->name, h._imp->extra_env))
{
}

Hook::~Hook()
{
}

Hook
Hook::operator() (const std::string & k, const std::string & v) const
{
    Hook result(*this);
    result._imp->extra_env.insert(std::make_pair(k, v));
    return result;
}

Hook::Iterator
Hook::begin() const
{
    return Iterator(_imp->extra_env.begin());
}

Hook::Iterator
Hook::end() const
{
    return Iterator(_imp->extra_env.end());
}

std::string
Hook::name() const
{
    return _imp->name;
}

