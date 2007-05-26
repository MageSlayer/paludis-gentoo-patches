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

#include "args_option.hh"
#include "args_error.hh"

#include <paludis/util/visitor.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/system.hh>
#include <paludis/util/join.hh>

#include <algorithm>
#include <sstream>
#include <stdlib.h>

/** \file
 * Implementation for ArgsVisitor.
 *
 * \ingroup grplibpaludisargs
 */

using namespace paludis;
using namespace paludis::args;

template class MutableVisitor<ArgsVisitorTypes>;
template class MutableAcceptInterface<ArgsVisitorTypes>;

template class MutableAcceptInterfaceVisitsThis<ArgsVisitorTypes, IntegerArg>;
template class MutableAcceptInterfaceVisitsThis<ArgsVisitorTypes, EnumArg>;
template class MutableAcceptInterfaceVisitsThis<ArgsVisitorTypes, StringArg>;
template class MutableAcceptInterfaceVisitsThis<ArgsVisitorTypes, StringSetArg>;
template class MutableAcceptInterfaceVisitsThis<ArgsVisitorTypes, AliasArg>;
template class MutableAcceptInterfaceVisitsThis<ArgsVisitorTypes, SwitchArg>;

template class Visits<IntegerArg>;
template class Visits<EnumArg>;
template class Visits<StringArg>;
template class Visits<StringSetArg>;
template class Visits<AliasArg>;
template class Visits<SwitchArg>;

template class Visits<const IntegerArg>;
template class Visits<const EnumArg>;
template class Visits<const StringArg>;
template class Visits<const StringSetArg>;
template class Visits<const AliasArg>;
template class Visits<const SwitchArg>;

ArgsVisitor::ArgsVisitor(libwrapiter::ForwardIterator<ArgsVisitor, std::string> * ai,
        libwrapiter::ForwardIterator<ArgsVisitor, std::string> ae,
        const std::string & env_prefix) :
    _args_index(ai),
    _args_end(ae),
    _env_prefix(env_prefix)
{
}

const std::string &
ArgsVisitor::get_param(const ArgsOption & arg)
{
    if (++(*_args_index) == _args_end)
        throw MissingValue("--" + arg.long_name());

    return **_args_index;
}

void ArgsVisitor::visit(StringArg & arg)
{
    arg.set_specified(true);

    if (! _env_prefix.empty())
        setenv(env_name(arg.long_name()).c_str(), "1", 1);

    std::string p(get_param(arg));
    arg.set_argument(p);

    if (! _env_prefix.empty())
        setenv(env_name(arg.long_name()).c_str(), p.c_str(), 1);
}

void ArgsVisitor::visit(AliasArg & arg)
{
    arg.other()->accept(*this);
}

void ArgsVisitor::visit(SwitchArg & arg)
{
    arg.set_specified(true);

    if (! _env_prefix.empty())
        setenv(env_name(arg.long_name()).c_str(), "1", 1);
}

void ArgsVisitor::visit(IntegerArg & arg)
{
    arg.set_specified(true);

    if (! _env_prefix.empty())
        setenv(env_name(arg.long_name()).c_str(), "1", 1);

    std::string param = get_param(arg);
    try
    {
        int a(destringify<int>(param));
        arg.set_argument(a);

        if (! _env_prefix.empty())
            setenv(env_name(arg.long_name()).c_str(), stringify(a).c_str(), 1);
    }
    catch (const DestringifyError &)
    {
        throw BadValue("--" + arg.long_name(), param);
    }
}

void ArgsVisitor::visit(EnumArg & arg)
{
    arg.set_specified(true);

    if (! _env_prefix.empty())
        setenv(env_name(arg.long_name()).c_str(), "1", 1);

    std::string p(get_param(arg));
    arg.set_argument(p);

    if (! _env_prefix.empty())
        setenv(env_name(arg.long_name()).c_str(), p.c_str(), 1);
}

void ArgsVisitor::visit(StringSetArg & arg)
{
    arg.set_specified(true);

    if (! _env_prefix.empty())
        setenv(env_name(arg.long_name()).c_str(), "1", 1);

    std::string param = get_param(arg);
    arg.add_argument(param);

    if (! _env_prefix.empty())
        setenv(env_name(arg.long_name()).c_str(), join(arg.begin_args(),
                    arg.end_args(), " ").c_str(), 1);
}

std::string
ArgsVisitor::env_name(const std::string & long_name) const
{
    std::string result(_env_prefix + "_" + long_name);
    std::replace(result.begin(), result.end(), '-', '_');
    return result;
}

