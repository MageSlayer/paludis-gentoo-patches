/* vim: set sw=4 sts=4 et foldmethod=syntax : */
/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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
#include "bad_value.hh"
#include "missing_value.hh"

#include <paludis/util/visitor.hh>
#include <paludis/util/destringify.hh>

#include <sstream>

/** \file
 * Implementation for ArgsVisitor.
 *
 * \ingroup grplibpaludisargs
 */

using namespace paludis;
using namespace args;

ArgsVisitor::ArgsVisitor(libwrapiter::ForwardIterator<ArgsVisitor, std::string> * ai,
        libwrapiter::ForwardIterator<ArgsVisitor, std::string> ae) : _args_index(ai), _args_end(ae)
{
}

const std::string &
ArgsVisitor::get_param(const ArgsOption * const arg)
{
    if (++(*_args_index) == _args_end)
    {
        throw MissingValue("--" + arg->long_name());
    }
    return **_args_index;
}

void ArgsVisitor::visit(ArgsOption * const arg)
{
    arg->set_specified(true);
}

void ArgsVisitor::visit(StringArg * const arg)
{
    visit(static_cast<ArgsOption *>(arg));
    arg->set_argument(get_param(arg));
}

void ArgsVisitor::visit(AliasArg * const arg)
{
    visit(static_cast<ArgsOption *>(arg));
}

void ArgsVisitor::visit(SwitchArg * const arg)
{
    visit(static_cast<ArgsOption *>(arg));
}

void ArgsVisitor::visit(IntegerArg * const arg)
{
    visit(static_cast<ArgsOption*>(arg));
    std::string param = get_param(arg);
    try
    {
        arg->set_argument(destringify<int>(param));
    }
    catch(DestringifyError &e)
    {
        throw BadValue("--" + arg->long_name(), param);
    }
}

void ArgsVisitor::visit(EnumArg * const arg)
{
    visit(static_cast<ArgsOption*>(arg));
    arg->set_argument(get_param(arg));
}

void ArgsVisitor::visit(StringSetArg * const arg)
{
    visit(static_cast<ArgsOption *>(arg));
    std::string param = get_param(arg);
    arg->add_argument(param);
}
