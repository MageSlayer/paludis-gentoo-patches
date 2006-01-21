/* vim: set sw=4 sts=4 et foldmethod=syntax : */

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

#include "args.hh"
#include "bad_value.hh"

/** \file
 * Implementation for ArgsOption.
 *
 * \ingroup Args
 */

using namespace paludis::args;

ArgsOption::ArgsOption(ArgsGroup * const g, const std::string & long_name,
        const char short_name, const std::string & description) :
    _group(g),
    _long_name(long_name),
    _short_name(short_name),
    _description(description),
    _specified(false)
{
    g->add(this);
    g->_handler->add_option(this, long_name, short_name);
}

ArgsOption::~ArgsOption()
{
}

StringArg::StringArg(ArgsGroup * const g, const std::string & long_name,
        const char short_name, const std::string & description) :
    ArgsOption(g, long_name, short_name, description)
{
}

IntegerArg::IntegerArg(ArgsGroup * const group, const std::string& long_name, 
                char short_name, const std::string& description) :
    ArgsOption(group, long_name, short_name, description)
{
}

void EnumArg::set_argument(const std::string & arg)
{
    if (_allowed_args.find(arg) == _allowed_args.end())
    {
        throw (BadValue("--" + long_name(), arg));
    }
    _argument = arg;
}

EnumArg::~EnumArg()
{
}

EnumArg::EnumArgOptions::EnumArgOptions(std::string opt, std::string desc)
{
    _options.insert(std::make_pair(opt, desc));
}

EnumArg::EnumArgOptions & EnumArg::EnumArgOptions::operator() (std::string opt, std::string desc)
{
    _options.insert(std::make_pair(opt, desc));
    return *this;
}

EnumArg::EnumArgOptions::~EnumArgOptions()
{
}
