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
#include "colours.hh"
#include "format_user_config.hh"
#include <paludis/util/stringify.hh>
#include <paludis/name.hh>
#include <paludis/choice.hh>

using namespace paludis;
using namespace cave;

ColourPrettyPrinter::ColourPrettyPrinter(
        const Environment * const env,
        const std::shared_ptr<const PackageID> & id,
        const int initial_indent) :
    FormattedPrettyPrinter(env, id),
    _initial_indent(initial_indent)
{
}

namespace
{
#include "colour_pretty_printer-fmt.hh"
}

const std::string
ColourPrettyPrinter::format_enabled(const std::string & v) const
{
    return fuc(fs_pretty_print_enabled(), fv<'s'>(v));
}

const std::string
ColourPrettyPrinter::format_disabled(const std::string & v) const
{
    return fuc(fs_pretty_print_disabled(), fv<'s'>(v));
}

const std::string
ColourPrettyPrinter::format_installed(const std::string & v) const
{
    return fuc(fs_pretty_print_installed(), fv<'s'>(v));
}

const std::string
ColourPrettyPrinter::format_installable(const std::string & v) const
{
    return fuc(fs_pretty_print_installable(), fv<'s'>(v));
}

const std::string
ColourPrettyPrinter::format_masked(const std::string & v) const
{
    return fuc(fs_pretty_print_masked(), fv<'s'>(v));
}

const std::string
ColourPrettyPrinter::format_plain(const std::string & v) const
{
    return fuc(fs_pretty_print_plain(), fv<'s'>(v));
}

const std::string
ColourPrettyPrinter::indentify(const int i) const
{
    return fuc(fs_pretty_print_indent(), fv<'i'>(std::string(_initial_indent + i, ' ')));
}

const std::string
ColourPrettyPrinter::newline() const
{
    return "\n";
}

const std::string
ColourPrettyPrinter::prettify_choice_value_forced(const std::shared_ptr<const ChoiceValue> & v) const
{
    return fuc(fs_pretty_print_choice_value_forced(),
            fv<'k'>(stringify(v->unprefixed_name())),
            fv<'v'>(v->parameter().empty() ? "" : "=" + v->parameter()));
}

const std::string
ColourPrettyPrinter::prettify_choice_value_enabled(const std::shared_ptr<const ChoiceValue> & v) const
{
    return fuc(fs_pretty_print_choice_value_enabled(),
            fv<'k'>(stringify(v->unprefixed_name())),
            fv<'v'>(v->parameter().empty() ? "" : "=" + v->parameter()));
}

const std::string
ColourPrettyPrinter::prettify_choice_value_masked(const std::shared_ptr<const ChoiceValue> & v) const
{
    return fuc(fs_pretty_print_choice_value_masked(),
            fv<'s'>(stringify(v->unprefixed_name())));
}

const std::string
ColourPrettyPrinter::prettify_choice_value_disabled(const std::shared_ptr<const ChoiceValue> & v) const
{
    return fuc(fs_pretty_print_choice_value_disabled(),
            fv<'s'>(stringify(v->unprefixed_name())));
}

