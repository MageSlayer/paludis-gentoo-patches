/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009 Ciaran McCreesh
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

#include "colour_formatter.hh"
#include "formats.hh"
#include "format_general.hh"
#include <paludis/util/stringify.hh>
#include <paludis/name.hh>
#include <paludis/choice.hh>

using namespace paludis;
using namespace cave;

ColourFormatter::ColourFormatter(const int initial_indent) :
    _initial_indent(initial_indent)
{
}

std::string
ColourFormatter::format(const paludis::ChoiceValue & s, const paludis::format::Plain &) const
{
    return format_general_s(f::colour_formatter_choice_value_plain(), stringify(s.name_with_prefix()));
}

std::string
ColourFormatter::format(const paludis::ChoiceValue & s, const paludis::format::Enabled &) const
{
    return format_general_kv(f::colour_formatter_choice_value_enabled(),
            stringify(s.unprefixed_name()), s.parameter().empty() ? "" : "=" + s.parameter());
}

std::string
ColourFormatter::format(const paludis::ChoiceValue & s, const paludis::format::Disabled &) const
{
    return format_general_s(f::colour_formatter_choice_value_disabled(), stringify(s.unprefixed_name()));
}

std::string
ColourFormatter::format(const paludis::ChoiceValue & s, const paludis::format::Forced &) const
{
    return format_general_kv(f::colour_formatter_choice_value_forced(),
            stringify(s.unprefixed_name()), s.parameter().empty() ? "" : "=" + s.parameter());
}

std::string
ColourFormatter::format(const paludis::ChoiceValue & s, const paludis::format::Masked &) const
{
    return format_general_s(f::colour_formatter_choice_value_masked(), stringify(s.unprefixed_name()));
}

std::string
ColourFormatter::decorate(const paludis::ChoiceValue &, const std::string & f, const paludis::format::Added &) const
{
    return f + "+";
}

std::string
ColourFormatter::decorate(const paludis::ChoiceValue &, const std::string & f, const paludis::format::Changed &) const
{
    return f + "*";
}

std::string
ColourFormatter::format(const KeywordName & s, const format::Plain &) const
{
    return format_general_s(f::colour_formatter_keyword_name_plain(), stringify(s));
}

std::string
ColourFormatter::format(const KeywordName & s, const format::Accepted &) const
{
    return format_general_s(f::colour_formatter_keyword_name_accepted(), stringify(s));
}

std::string
ColourFormatter::format(const KeywordName & s, const format::Unaccepted &) const
{
    return format_general_s(f::colour_formatter_keyword_name_unaccepted(), stringify(s));
}

std::string
ColourFormatter::format(const std::string & s, const format::Plain &) const
{
    return format_general_s(f::colour_formatter_string_plain(), stringify(s));
}

std::string
ColourFormatter::format(const PackageID & s, const format::Plain &) const
{
    return format_general_s(f::colour_formatter_package_id_plain(), stringify(s));
}

std::string
ColourFormatter::format(const PackageID & s, const format::Installed &) const
{
    return format_general_s(f::colour_formatter_package_id_installed(), stringify(s));
}

std::string
ColourFormatter::format(const PackageID & s, const format::Installable &) const
{
    return format_general_s(f::colour_formatter_package_id_installable(), stringify(s));
}

std::string
ColourFormatter::format(const LicenseDepSpec & s, const format::Plain &) const
{
    return format_general_s(f::colour_formatter_license_dep_spec_plain(), stringify(s));
}

std::string
ColourFormatter::format(const LicenseDepSpec & s, const format::Accepted &) const
{
    return format_general_s(f::colour_formatter_license_dep_spec_accepted(), stringify(s));
}

std::string
ColourFormatter::format(const LicenseDepSpec & s, const format::Unaccepted &) const
{
    return format_general_s(f::colour_formatter_license_dep_spec_unaccepted(), stringify(s));
}

std::string
ColourFormatter::format(const ConditionalDepSpec & s, const format::Plain &) const
{
    return format_general_s(f::colour_formatter_conditional_dep_spec_plain(), stringify(s));
}

std::string
ColourFormatter::format(const ConditionalDepSpec & s, const format::Enabled &) const
{
    return format_general_s(f::colour_formatter_conditional_dep_spec_enabled(), stringify(s));
}

std::string
ColourFormatter::format(const ConditionalDepSpec & s, const format::Disabled &) const
{
    return format_general_s(f::colour_formatter_conditional_dep_spec_disabled(), stringify(s));
}

std::string
ColourFormatter::format(const ConditionalDepSpec & s, const format::Forced &) const
{
    return format_general_s(f::colour_formatter_conditional_dep_spec_forced(), stringify(s));
}

std::string
ColourFormatter::format(const ConditionalDepSpec & s, const format::Masked &) const
{
    return format_general_s(f::colour_formatter_conditional_dep_spec_masked(), stringify(s));
}

std::string
ColourFormatter::decorate(const ConditionalDepSpec &, const std::string & s, const format::Added &) const
{
    return s;
}

std::string
ColourFormatter::decorate(const ConditionalDepSpec &, const std::string & s, const format::Changed &) const
{
    return s;
}

std::string
ColourFormatter::format(const PlainTextDepSpec & s, const format::Plain &) const
{
    return format_general_s(f::colour_formatter_plain_text_dep_spec_plain(), stringify(s));
}

std::string
ColourFormatter::format(const SimpleURIDepSpec & s, const format::Plain &) const
{
    return format_general_s(f::colour_formatter_simple_uri_dep_spec_plain(), stringify(s));
}

std::string
ColourFormatter::format(const FetchableURIDepSpec & s, const format::Plain &) const
{
    return format_general_s(f::colour_formatter_fetchable_uri_dep_spec_plain(), stringify(s));
}

std::string
ColourFormatter::format(const URILabelsDepSpec & s, const format::Plain &) const
{
    return format_general_s(f::colour_formatter_uri_labels_dep_spec_plain(), stringify(s));
}

std::string
ColourFormatter::format(const PlainTextLabelDepSpec & s, const format::Plain &) const
{
    return format_general_s(f::colour_formatter_uri_labels_dep_spec_plain(), stringify(s));
}

std::string
ColourFormatter::format(const PackageDepSpec & s, const format::Plain &) const
{
    return format_general_s(f::colour_formatter_package_dep_spec_plain(), stringify(s));
}

std::string
ColourFormatter::format(const PackageDepSpec & s, const format::Installed &) const
{
    return format_general_s(f::colour_formatter_package_dep_spec_installed(), stringify(s));
}

std::string
ColourFormatter::format(const PackageDepSpec & s, const format::Installable &) const
{
    return format_general_s(f::colour_formatter_package_dep_spec_installable(), stringify(s));
}

std::string
ColourFormatter::format(const DependenciesLabelsDepSpec & s, const format::Plain &) const
{
    return format_general_s(f::colour_formatter_dependency_labels_dep_spec_plain(), stringify(s));
}

std::string
ColourFormatter::format(const BlockDepSpec & s, const format::Plain &) const
{
    return format_general_s(f::colour_formatter_block_dep_spec_plain(), stringify(s));
}

std::string
ColourFormatter::format(const NamedSetDepSpec & s, const format::Plain &) const
{
    return format_general_s(f::colour_formatter_named_set_dep_spec_plain(), stringify(s));
}

std::string
ColourFormatter::newline() const
{
    return "\n";
}

std::string
ColourFormatter::indent(const int i) const
{
    return format_general_i(f::colour_formatter_indent(), i);
}

