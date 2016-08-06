/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
 * Copyright (c) 2007 Piotr Jaroszyński
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

#include <paludis/hook.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <utility>
#include <map>
#include <set>

using namespace paludis;

#include <paludis/hook-se.cc>

namespace paludis
{
    template <>
    struct Imp<Hook>
    {
        std::string name;
        std::map<std::string, std::string> extra_env;
        std::set<std::string> allowed_values;

        Imp(const std::string & n, const std::map<std::string, std::string> & e,
                const std::set<std::string> & av) :
            name(n),
            extra_env(e),
            allowed_values(av)
        {
        }
    };

    template <>
    struct Imp<Hook::AllowedOutputValues>
    {
        std::set<std::string> allowed_values;
    };

    template <>
    struct WrappedForwardIteratorTraits<Hook::ConstIteratorTag>
    {
        typedef std::map<std::string, std::string>::const_iterator UnderlyingIterator;
    };
}

Hook::AllowedOutputValues::AllowedOutputValues() :
    _imp()
{
}

Hook::AllowedOutputValues::AllowedOutputValues(const AllowedOutputValues & other) :
    _imp()
{
    _imp->allowed_values = other._imp->allowed_values;
}

Hook::AllowedOutputValues::~AllowedOutputValues() = default;

Hook::AllowedOutputValues
Hook::AllowedOutputValues::operator() (const std::string & v) const
{
    AllowedOutputValues result(*this);
    result._imp->allowed_values.insert(v);
    return result;
}

Hook::Hook(const std::string & n) :
    _imp(n, std::map<std::string, std::string>(), std::set<std::string>()),
    output_dest(hod_stdout)
{
}

Hook::Hook(const Hook & h) :
    _imp(h._imp->name, h._imp->extra_env, h._imp->allowed_values),
    output_dest(h.output_dest)
{
}

Hook::~Hook() = default;

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
    result._imp->allowed_values = av._imp->allowed_values;
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

Hook::ConstIterator
Hook::begin() const
{
    return ConstIterator(_imp->extra_env.begin());
}

Hook::ConstIterator
Hook::end() const
{
    return ConstIterator(_imp->extra_env.end());
}

std::string
Hook::name() const
{
    return _imp->name;
}

namespace paludis
{
    template class WrappedForwardIterator<Hook::ConstIteratorTag, const std::pair<const std::string, std::string> >;
}
