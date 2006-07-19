/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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
 * \ingroup grplibpaludisargs
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
    g->handler()->add_option(this, long_name, short_name);
}

ArgsOption::~ArgsOption()
{
}

SwitchArg::SwitchArg(ArgsGroup * const group, std::string long_name, char short_name,
        std::string description) :
    ArgsOption(group, long_name, short_name, description)
{
}

SwitchArg::~SwitchArg()
{
}

AliasArg::AliasArg(ArgsOption * const other, const std::string & long_name) :
    ArgsOption(other->group(), long_name, '\0', "Alias for --" + other->long_name()),
    _other(other)
{
    other->group()->handler()->add_option(other, long_name);
}

StringArg::StringArg(ArgsGroup * const g, const std::string & long_name,
        const char short_name, const std::string & description) :
    ArgsOption(g, long_name, short_name, description)
{
}

StringSetArg::StringSetArg(ArgsGroup * const g, const std::string & long_name,
        const char short_name, const std::string & description) :
    ArgsOption(g, long_name, short_name, description)
{
}

IntegerArg::IntegerArg(ArgsGroup * const group, const std::string& long_name, 
                char short_name, const std::string& description) :
    ArgsOption(group, long_name, short_name, description)
{
}

namespace
{
    /**
     * Is an arg a particular value?
     *
     * \ingroup grplibpaludisargs
     */
    struct ArgIs
    {
        /// The argument.
        const std::string arg;

        /// Constructor.
        ArgIs(const std::string & a) :
            arg(a)
        {
        }

        /// Comparator.
        bool operator() (const std::pair<std::string, std::string> & p) const
        {
            return p.first == arg;
        }
    };
}

void EnumArg::set_argument(const std::string & arg)
{
    if (_allowed_args.end() == std::find_if(_allowed_args.begin(),
                _allowed_args.end(), ArgIs(arg)))
        throw (BadValue("--" + long_name(), arg));

    _argument = arg;
}

EnumArg::~EnumArg()
{
}

EnumArg::EnumArgOptions::EnumArgOptions(std::string opt, std::string desc)
{
    _options.push_back(std::make_pair(opt, desc));
}

EnumArg::EnumArgOptions & EnumArg::EnumArgOptions::operator() (std::string opt, std::string desc)
{
    _options.push_back(std::make_pair(opt, desc));
    return *this;
}

EnumArg::EnumArgOptions::~EnumArgOptions()
{
}
