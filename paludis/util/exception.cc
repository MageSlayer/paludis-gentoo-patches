/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2010 Ciaran McCreesh
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

#include <paludis/util/exception.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/join.hh>
#include <memory>
#include <list>
#include <cstdlib>
#include <iostream>

#include "config.h"

#ifdef HAVE_CXA_DEMANGLE
#  include <cxxabi.h>
#endif

using namespace paludis;

namespace
{
    static thread_local std::list<std::string> context;
}

Context::Context(const std::string & s)
{
    context.push_back(s);
}

Context::~Context() noexcept(false)
{
    if (context.empty())
        throw InternalError(PALUDIS_HERE, "no context");
    context.pop_back();
}

std::string
Context::backtrace(const std::string & delim)
{
    if (context.empty())
        return "";

    return join(context.begin(), context.end(), delim) + delim;
}

namespace paludis
{
    struct Exception::ContextData
    {
        std::list<std::string> local_context;

        ContextData()
        {
            local_context.assign(context.begin(), context.end());
        }

        ContextData(const ContextData & other) = default;
    };
}

Exception::Exception(const std::string & m) noexcept :
    _message(m),
    _context_data(std::make_unique<ContextData>())
{
}

Exception::Exception(const Exception & other) :
    std::exception(other),
    _message(other._message),
    _context_data(std::make_unique<ContextData>(*other._context_data))
{
}

Exception::~Exception() = default;

const std::string &
Exception::message() const noexcept
{
    return _message;
}

std::string
Exception::backtrace(const std::string & delim) const
{
    return join(_context_data->local_context.begin(), _context_data->local_context.end(), delim) + delim;
}

bool
Exception::empty() const
{
    return _context_data->local_context.empty();
}

NotAvailableError::NotAvailableError(const std::string & msg) noexcept :
    Exception("Error: Not available: " + msg)
{
}

InternalError::InternalError(const std::string & location, const std::string & our_message) noexcept :
    Exception("Eek! Internal error at " + location + ": " + our_message)
{
    std::cerr << "Internal error at " << location << ": " << our_message << std::endl;
}

NameError::NameError(const std::string & name, const std::string & role) noexcept :
    Exception("Name '" + name + "' is not a valid " + role)
{
}

NameError::NameError(const std::string & name, const std::string & role, const std::string & msg) noexcept :
    Exception("Name '" + name + "' is not a valid " + role + ": " + msg)
{
}

ConfigurationError::ConfigurationError(const std::string & msg) noexcept :
    Exception(msg)
{
}

const char *
Exception::what() const noexcept
{
#ifdef HAVE_CXA_DEMANGLE
    if (_what_str.empty())
    {
        int status(0);
        char * const name(abi::__cxa_demangle(
                    (std::string("_Z") + typeid(*this).name()).c_str(), nullptr, nullptr, &status));

        if (0 == status)
        {
            _what_str = name;
            std::free(name);
        }
    }
#endif

    if (_what_str.empty())
        _what_str = stringify(std::exception::what());

    return _what_str.c_str();
}

