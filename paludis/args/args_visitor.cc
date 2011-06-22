/* vim: set sw=4 sts=4 et foldmethod=syntax : */
/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#include "args_visitor.hh"
#include "args_option.hh"
#include "args_error.hh"
#include "args_handler.hh"
#include "bad_argument.hh"

#include <paludis/util/destringify.hh>
#include <paludis/util/system.hh>
#include <paludis/util/join.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/log.hh>

#include <algorithm>
#include <sstream>
#include <stdlib.h>

using namespace paludis;
using namespace paludis::args;

namespace paludis
{
    template <>
    struct Imp<ArgsVisitor>
    {
        ArgsHandler::ArgsIterator * args_index;
        ArgsHandler::ArgsIterator args_end;
        std::string env_prefix;
        std::string & remaining_chars;
        bool no;
        ArgsOptionSpecifiedness specifiedness;

        Imp(
                ArgsHandler::ArgsIterator * i,
                ArgsHandler::ArgsIterator e,
                std::string p,
                std::string & s,
                bool n,
                ArgsOptionSpecifiedness sp) :
            args_index(i),
            args_end(e),
            env_prefix(p),
            remaining_chars(s),
            no(n),
            specifiedness(sp)
        {
        }
    };
}

ArgsVisitor::ArgsVisitor(ArgsHandler::ArgsIterator * ai, ArgsHandler::ArgsIterator ae,
        const std::string & env_prefix, std::string & s, bool n, ArgsOptionSpecifiedness p) :
    _imp(ai, ae, env_prefix, s, n, p)
{
}

ArgsVisitor::~ArgsVisitor()
{
}

const std::string &
ArgsVisitor::get_param(const ArgsOption & arg)
{
    if (++(*_imp->args_index) == _imp->args_end)
        throw MissingValue("--" + arg.long_name());

    return **_imp->args_index;
}

void ArgsVisitor::visit(StringArg & arg)
{
    if (! _imp->no)
    {
        if (arg.explicitly_specified())
            Log::get_instance()->message("args.specified_twice", ll_warning, lc_context)
                <<  "Option '--" << arg.long_name() << "' was specified more than once, but it does not take multiple values";

        std::string p(get_param(arg));
        arg.set_specified(_imp->specifiedness);
        arg.set_argument(p);
        if (! _imp->env_prefix.empty())
            setenv(env_name(arg.long_name()).c_str(), p.c_str(), 1);
    }
    else if (! arg.can_be_negated())
        throw BadArgument("--no-" + arg.long_name());
    else
    {
        arg.set_specified(aos_not);
        if (! _imp->env_prefix.empty())
            unsetenv(env_name(arg.long_name()).c_str());
    }
}

void ArgsVisitor::visit(AliasArg & arg)
{
    arg.other()->accept(*this);
}

void ArgsVisitor::visit(SwitchArg & arg)
{
    if (! _imp->no)
    {
        arg.set_specified(_imp->specifiedness);
        if (! _imp->env_prefix.empty())
            setenv(env_name(arg.long_name()).c_str(), "1", 1);
    }
    else if (! arg.can_be_negated())
        throw BadArgument("--no-" + arg.long_name());
    else
    {
        arg.set_specified(aos_not);
        if (! _imp->env_prefix.empty())
            unsetenv(env_name(arg.long_name()).c_str());
    }
}

void ArgsVisitor::visit(IntegerArg & arg)
{
    if (! _imp->no)
    {
        if (arg.explicitly_specified())
            Log::get_instance()->message("args.specified_twice", ll_warning, lc_context)
                <<  "Option '--" << arg.long_name() << "' was specified more than once, but it does not take multiple values";

        arg.set_specified(_imp->specifiedness);
        std::string param;

        if ((! _imp->remaining_chars.empty()) && (std::string::npos == _imp->remaining_chars.find_first_not_of("0123456789")))
        {
            param = _imp->remaining_chars;
            _imp->remaining_chars.clear();
        }
        else
            param = get_param(arg);

        try
        {
            int a(destringify<int>(param));
            arg.set_argument(a);

            if (! _imp->env_prefix.empty())
                setenv(env_name(arg.long_name()).c_str(), stringify(a).c_str(), 1);
        }
        catch (const DestringifyError &)
        {
            throw BadValue("--" + arg.long_name(), param);
        }
    }
    else
        throw BadArgument("--no-" + arg.long_name());
}

void ArgsVisitor::visit(EnumArg & arg)
{
    if (! _imp->no)
    {
        if (arg.explicitly_specified())
            Log::get_instance()->message("args.specified_twice", ll_warning, lc_context)
                <<  "Option '--" << arg.long_name() << "' was specified more than once, but it does not take multiple values";

        arg.set_specified(_imp->specifiedness);

        std::string p;
        if (_imp->remaining_chars.length() == 1)
        {
            p = _imp->remaining_chars;
            _imp->remaining_chars.clear();
        }
        else
            p = get_param(arg);

        arg.set_argument(p);
        if (! _imp->env_prefix.empty())
            setenv(env_name(arg.long_name()).c_str(), p.c_str(), 1);
    }
    else
        throw BadArgument("--no-" + arg.long_name());
}

void ArgsVisitor::visit(StringSetArg & arg)
{
    if (! _imp->no)
    {
        arg.set_specified(_imp->specifiedness);

        std::string param = get_param(arg);
        arg.add_argument(param);

        if (! _imp->env_prefix.empty())
            setenv(env_name(arg.long_name()).c_str(), join(arg.begin_args(),
                        arg.end_args(), " ").c_str(), 1);
    }
    else
        throw BadArgument("--no-" + arg.long_name());
}

void ArgsVisitor::visit(StringSequenceArg & arg)
{
    if (! _imp->no)
    {
        arg.set_specified(_imp->specifiedness);

        std::string param = get_param(arg);
        arg.add_argument(param);

        if (! _imp->env_prefix.empty())
            setenv(env_name(arg.long_name()).c_str(), join(arg.begin_args(),
                        arg.end_args(), " ").c_str(), 1);
    }
    else
        throw BadArgument("--no-" + arg.long_name());
}

std::string
ArgsVisitor::env_name(const std::string & long_name) const
{
    std::string result(_imp->env_prefix + "_" + long_name);
    std::replace(result.begin(), result.end(), '-', '_');
    return result;
}

