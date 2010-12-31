/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Ciaran McCreesh
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

#include "colour_pretty_printer.hh"
#include "colour.hh"
#include <paludis/choice.hh>
#include <paludis/name.hh>

using namespace paludis;

ColourPrettyPrinter::ColourPrettyPrinter(const Environment * const e, const std::shared_ptr<const PackageID> & i) :
    FormattedPrettyPrinter(e, i)
{
}

ColourPrettyPrinter::~ColourPrettyPrinter() = default;

const std::string
ColourPrettyPrinter::format_enabled(const std::string & s) const
{
    return colour(cl_flag_on, s);
}

const std::string
ColourPrettyPrinter::format_disabled(const std::string & s) const
{
    return colour(cl_flag_off, s);
}

const std::string
ColourPrettyPrinter::format_installed(const std::string & s) const
{
    return colour(cl_none, s);
}

const std::string
ColourPrettyPrinter::format_installable(const std::string & s) const
{
    return colour(cl_installable_package_name, s);
}

const std::string
ColourPrettyPrinter::format_masked(const std::string & s) const
{
    return colour(cl_masked, s);
}

const std::string
ColourPrettyPrinter::format_plain(const std::string & s) const
{
    return colour(cl_none, s);
}

const std::string
ColourPrettyPrinter::indentify(const int i) const
{
    return std::string(12 + (4 * i), ' ');
}

const std::string
ColourPrettyPrinter::newline() const
{
    return "\n";
}

const std::string
ColourPrettyPrinter::prettify(const std::shared_ptr<const ChoiceValue> & v) const
{
    if (v->enabled())
    {
        if (v->locked())
        {
            std::string s(colour(cl_flag_on, "(" + stringify(v->unprefixed_name())));
            if (! v->parameter().empty())
                s.append("=" + v->parameter());
            return s + colour(cl_flag_on, ")");
        }
        else
        {
            std::string s(colour(cl_flag_on, stringify(v->unprefixed_name())));
            if (! v->parameter().empty())
                s.append("=" + v->parameter());
            return s;
        }
    }
    else
    {
        if (v->locked())
            return colour(cl_flag_off, "(-" + stringify(v->unprefixed_name()) + ")");
        else
            return colour(cl_flag_off, "-" + stringify(v->unprefixed_name()));
    }
}

