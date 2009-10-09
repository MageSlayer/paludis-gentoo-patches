/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009 Ciaran McCreesh
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
#include "colour.hh"
#include <paludis/util/stringify.hh>
#include <paludis/util/set.hh>
#include <paludis/name.hh>
#include <paludis/choice.hh>

using namespace paludis;

std::string
ColourFormatter::format(const ChoiceValue & f, const format::Plain &) const
{
    return stringify(f.name_with_prefix());
}

std::string
ColourFormatter::format(const ChoiceValue & f, const format::Enabled &) const
{
    std::string s(colour(cl_flag_on, stringify(f.unprefixed_name())));
    if (! f.parameter().empty())
        s.append("=" + f.parameter());
    return s;
}

std::string
ColourFormatter::format(const ChoiceValue & f, const format::Disabled &) const
{
    return colour(cl_flag_off, "-" + stringify(f.unprefixed_name()));
}

std::string
ColourFormatter::format(const ChoiceValue & f, const format::Forced &) const
{
    std::string s(colour(cl_flag_on, "(" + stringify(f.unprefixed_name())));
    if (! f.parameter().empty())
        s.append("=" + f.parameter());
    return s + colour(cl_flag_on, ")");
}

std::string
ColourFormatter::format(const ChoiceValue & f, const format::Masked &) const
{
    return colour(cl_flag_off, "(-" + stringify(f.unprefixed_name()) + ")");
}

std::string
ColourFormatter::decorate(const ChoiceValue &, const std::string & f, const format::Added &) const
{
    return f + "+";
}

std::string
ColourFormatter::decorate(const ChoiceValue &, const std::string & f, const format::Changed &) const
{
    return f + "*";
}

std::string
ColourFormatter::format(const ConditionalDepSpec & f, const format::Plain &) const
{
    return stringify(f);
}

std::string
ColourFormatter::format(const ConditionalDepSpec & f, const format::Enabled &) const
{
    return colour(cl_flag_on, f);
}

std::string
ColourFormatter::format(const ConditionalDepSpec & f, const format::Disabled &) const
{
    return colour(cl_flag_off, f);
}

std::string
ColourFormatter::format(const ConditionalDepSpec & f, const format::Forced &) const
{
    return colour(cl_flag_on, "(" + stringify(f) + ")");
}

std::string
ColourFormatter::format(const ConditionalDepSpec & f, const format::Masked &) const
{
    return colour(cl_flag_off, "(" + stringify(f) + ")");
}

std::string
ColourFormatter::decorate(const ConditionalDepSpec &, const std::string & f, const format::Added &) const
{
    return f;
}

std::string
ColourFormatter::decorate(const ConditionalDepSpec &, const std::string & f, const format::Changed &) const
{
    return f;
}

std::string
ColourFormatter::format(const PackageDepSpec & f, const format::Plain &) const
{
    return stringify(f);
}

std::string
ColourFormatter::format(const PackageDepSpec & f, const format::Installed &) const
{
    return colour(cl_package_name, f);
}

std::string
ColourFormatter::format(const PackageDepSpec & f, const format::Installable &) const
{
    return colour(cl_installable_package_name, f);
}

std::string
ColourFormatter::format(const PlainTextDepSpec & f, const format::Plain &) const
{
    return stringify(f);
}

std::string
ColourFormatter::format(const LicenseDepSpec & f, const format::Plain &) const
{
    return stringify(f);
}

std::string
ColourFormatter::format(const LicenseDepSpec & f, const format::Accepted &) const
{
    return colour(cl_flag_on, f);
}

std::string
ColourFormatter::format(const LicenseDepSpec & f, const format::Unaccepted &) const
{
    return colour(cl_flag_off, f);
}

std::string
ColourFormatter::format(const KeywordName & f, const format::Plain &) const
{
    return stringify(f);
}

std::string
ColourFormatter::format(const KeywordName & f, const format::Accepted &) const
{
    return colour(cl_flag_on, f);
}

std::string
ColourFormatter::format(const KeywordName & f, const format::Unaccepted &) const
{
    return colour(cl_flag_off, f);
}

std::string
ColourFormatter::format(const std::string & f, const format::Plain &) const
{
    return stringify(f);
}

std::string
ColourFormatter::format(const PlainTextLabelDepSpec & f, const format::Plain &) const
{
    return stringify(f);
}

std::string
ColourFormatter::format(const URILabelsDepSpec & f, const format::Plain &) const
{
    return stringify(f);
}

std::string
ColourFormatter::format(const NamedSetDepSpec & f, const format::Plain &) const
{
    return stringify(f);
}

std::string
ColourFormatter::format(const DependenciesLabelsDepSpec & f, const format::Plain &) const
{
    return stringify(f);
}

std::string
ColourFormatter::format(const FetchableURIDepSpec & f, const format::Plain &) const
{
    return stringify(f);
}

std::string
ColourFormatter::format(const SimpleURIDepSpec & f, const format::Plain &) const
{
    return stringify(f);
}

std::string
ColourFormatter::format(const BlockDepSpec & f, const format::Plain &) const
{
    return stringify(f);
}

std::string
ColourFormatter::format(const PackageID & f, const format::Plain &) const
{
    return stringify(f);
}

std::string
ColourFormatter::format(const PackageID & f, const format::Installed &) const
{
    return colour(cl_package_name, f);
}

std::string
ColourFormatter::format(const PackageID & f, const format::Installable &) const
{
    return colour(cl_installable_package_name, f);
}

std::string
ColourFormatter::format(const FSEntry & f, const format::Plain &) const
{
    return stringify(f);
}

std::string
ColourFormatter::newline() const
{
    return "\n";
}

std::string
ColourFormatter::indent(const int i) const
{
    return std::string(12 + (4 * i), ' ');
}

