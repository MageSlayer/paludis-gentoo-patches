/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Stephen Bennett
 *
 * This file is part of the Paludis package manager. Paludis is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License, version 2, as published by the Free Software Foundation.
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

#include <paludis/args/args_dumper.hh>
#include <paludis/args/args_option.hh>

#include <sstream>

using namespace paludis;
using namespace paludis::args;

ArgsDumper::ArgsDumper(std::ostream & os) :
    _os(os)
{
}

void ArgsDumper::generic_visit(const ArgsOption & a)
{
    std::stringstream p;
    p << "  --" << a.long_name();
    if (a.short_name())
        p << ", -" << a.short_name();
    if (p.str().length() < 24)
        p << std::string(24 - p.str().length(), ' ');
    else
        p << std::endl << std::string(24, ' ');
    _os << p.str();
    _os << " " << a.description() << std::endl;
}

void ArgsDumper::visit(const StringSetArg & a)
{
    generic_visit(a);

    for (const auto & arg : a.allowed_args())
    {
        std::stringstream p;
        p << "      " << arg.first;
        if (p.str().length() < 26)
            p << std::string(26 - p.str().length(), ' ');
        _os << p.str();
        _os << " " << arg.second;
        _os << std::endl;
    }
}

void ArgsDumper::visit(const StringSequenceArg & a)
{
    generic_visit(a);
}

void ArgsDumper::visit(const EnumArg & a)
{
    generic_visit(a);

    for (const auto & arg : a.allowed_args())
    {
        std::stringstream p;
        p << "      " << arg.long_name();
        if (arg.short_name())
            p << " (" << std::string(1, arg.short_name()) << ")";
        if (p.str().length() < 26)
            p << std::string(26 - p.str().length(), ' ');
        _os << p.str();
        _os << " " << arg.description();
        if (arg.long_name() == a.default_arg())
            _os << " (default)";
        _os << std::endl;
    }
}

void
ArgsDumper::visit(const SwitchArg & a)
{
    generic_visit(a);
}

void
ArgsDumper::visit(const StringArg & a)
{
    generic_visit(a);
}

void
ArgsDumper::visit(const IntegerArg & a)
{
    generic_visit(a);
}

void
ArgsDumper::visit(const AliasArg & a)
{
    if(! a.hidden())
        generic_visit(a);
}

