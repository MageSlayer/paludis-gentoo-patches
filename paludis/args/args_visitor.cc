#include "args_visitor.hh"
#include "args_option.hh"
#include "alias_arg.hh"
#include "switch_arg.hh"

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#include "bad_value.hh"

#include <sstream>

using namespace paludis;
using namespace args;

ArgsVisitor::ArgsVisitor(std::list<std::string>::iterator *ai) : _args_index(ai)
{
}

void ArgsVisitor::visit(ArgsOption * const arg)
{
    arg->set_specified(true);
}

void ArgsVisitor::visit(StringArg * const arg)
{
    visit(static_cast<ArgsOption *>(arg));
    arg->set_argument(*++(*_args_index));
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
    std::stringstream ss;
    std::string param =  *++(*_args_index);
    ss << param;
    int i;
    ss >> i;
    if (!ss.eof() || ss.bad())
        throw BadValue("--" + arg->long_name(), param);
    arg->set_argument(i);
}

void ArgsVisitor::visit(EnumArg * const arg)
{
    visit(static_cast<ArgsOption*>(arg));
    arg->set_argument(*++(*_args_index));
}
