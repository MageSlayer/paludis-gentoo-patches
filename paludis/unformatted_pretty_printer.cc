/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2011 Ciaran McCreesh
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

#include <paludis/unformatted_pretty_printer.hh>
#include <paludis/util/stringify.hh>
#include <paludis/name.hh>
#include <paludis/maintainer.hh>

using namespace paludis;

UnformattedPrettyPrinter::UnformattedPrettyPrinter() = default;

const std::string
UnformattedPrettyPrinter::indentify(const int i) const
{
    return std::string(i * 4, ' ');
}

const std::string
UnformattedPrettyPrinter::newline() const
{
    return "\n";
}

const std::string
UnformattedPrettyPrinter::prettify(const PackageDepSpec & v) const
{
    return stringify(v);
}

const std::string
UnformattedPrettyPrinter::prettify(const BlockDepSpec & v) const
{
    return stringify(v);
}

const std::string
UnformattedPrettyPrinter::prettify(const ConditionalDepSpec & v) const
{
    return stringify(v);
}

const std::string
UnformattedPrettyPrinter::prettify(const NamedSetDepSpec & v) const
{
    return stringify(v);
}

const std::string
UnformattedPrettyPrinter::prettify(const SimpleURIDepSpec & v) const
{
    return stringify(v);
}

const std::string
UnformattedPrettyPrinter::prettify(const PlainTextDepSpec & v) const
{
    return stringify(v);
}

const std::string
UnformattedPrettyPrinter::prettify(const LicenseDepSpec & v) const
{
    return stringify(v);
}

const std::string
UnformattedPrettyPrinter::prettify(const FetchableURIDepSpec & v) const
{
    return stringify(v);
}

const std::string
UnformattedPrettyPrinter::prettify(const URILabelsDepSpec & v) const
{
    return stringify(v);
}

const std::string
UnformattedPrettyPrinter::prettify(const DependenciesLabelsDepSpec & v) const
{
    return stringify(v);
}

const std::string
UnformattedPrettyPrinter::prettify(const PlainTextLabelDepSpec & v) const
{
    return stringify(v);
}

const std::string
UnformattedPrettyPrinter::prettify(const std::shared_ptr<const PackageID> & v) const
{
    return stringify(*v);
}

const std::string
UnformattedPrettyPrinter::prettify(const bool v) const
{
    return stringify(v);
}

const std::string
UnformattedPrettyPrinter::prettify(const long v) const
{
    return stringify(v);
}

const std::string
UnformattedPrettyPrinter::prettify(const std::string & v) const
{
    return stringify(v);
}

const std::string
UnformattedPrettyPrinter::prettify(const std::pair<const std::string, std::string> & v) const
{
    if (v.first.empty())
        return v.second;
    else
        return v.first + ": " + v.second;
}

const std::string
UnformattedPrettyPrinter::prettify(const FSPath & v) const
{
    return stringify(v);
}

const std::string
UnformattedPrettyPrinter::prettify(const KeywordName & v) const
{
    return stringify(v);
}

const std::string
UnformattedPrettyPrinter::prettify(const Maintainer & v) const
{
    return stringify(v);
}

