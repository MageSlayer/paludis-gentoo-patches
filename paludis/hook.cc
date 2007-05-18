/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
 * Copyright (c) 2007 Piotr Jaroszyński <peper@gentoo.org>
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

#include <paludis/hook-se.cc>
#include <paludis/hook-sr.cc>

namespace paludis
{
    template<>
    struct Implementation<Hook>
    {
        std::string name;
        std::map<std::string, std::string> extra_env;
        std::set<std::string> allowed_values;

        Implementation(const std::string & n, const std::map<std::string, std::string> & e,
                const std::set<std::string> & av) :
            name(n),
            extra_env(e),
            allowed_values(av)
        {
        }
    };
}

Hook::Hook(const std::string & n) :
    PrivateImplementationPattern<Hook>(new Implementation<Hook>(n, std::map<std::string, std::string>(),
                std::set<std::string>())),
    output_dest(hod_stdout)
{
}

Hook::Hook(const Hook & h) :
    PrivateImplementationPattern<Hook>(new Implementation<Hook>(h._imp->name, h._imp->extra_env,
                h._imp->allowed_values)),
    output_dest(h.output_dest)
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

std::string
Hook::get(const std::string & k) const
{
    std::map<std::string, std::string>::const_iterator i(_imp->extra_env.find(k));
     if (i != _imp->extra_env.end())
         return i->second;
     else
         return std::string("");
}

Hook
Hook::grab_output(const AllowedOutputValues & av)
{
    Hook result(*this);
    result.output_dest = hod_grab;
    result._imp->allowed_values = av.allowed_values;
    return result;
}

bool
Hook::validate_value(const std::string & v) const
{
    if (_imp->allowed_values.empty() || v.empty())
        return true;
    else
        return (_imp->allowed_values.find(v) != _imp->allowed_values.end());
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

Hook::AllowedOutputValues::AllowedOutputValues()
{
}

Hook::AllowedOutputValues::AllowedOutputValues(const AllowedOutputValues & other) :
    allowed_values(other.allowed_values)
{
}

Hook::AllowedOutputValues::~AllowedOutputValues()
{
}

Hook::AllowedOutputValues
Hook::AllowedOutputValues::operator() (const std::string & v) const
{
    AllowedOutputValues result(*this);
    result.allowed_values.insert(v);
    return result;
}
