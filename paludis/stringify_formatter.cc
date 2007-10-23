/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#include <paludis/stringify_formatter.hh>
#include <paludis/stringify_formatter-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/dep_spec.hh>
#include <paludis/dep_label.hh>

using namespace paludis;

StringifyFormatter::StringifyFormatter() :
    PrivateImplementationPattern<StringifyFormatter>(new Implementation<StringifyFormatter>(
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0))
{
}

StringifyFormatter::~StringifyFormatter()
{
}

std::string
StringifyFormatter::format(const std::string & s, const format::Plain & k) const
{
    if (_imp->f_str)
        return _imp->f_str->format(s, k);
    return s;
}

std::string
StringifyFormatter::format(const UseFlagName & s, const format::Enabled & k) const
{
    if (_imp->f_use)
        return _imp->f_use->format(s, k);
    return stringify(s);
}

std::string
StringifyFormatter::format(const UseFlagName & s, const format::Disabled & k) const
{
    if (_imp->f_use)
        return _imp->f_use->format(s, k);
    return stringify(s);
}

std::string
StringifyFormatter::format(const UseFlagName & s, const format::Forced & k) const
{
    if (_imp->f_use)
        return _imp->f_use->format(s, k);
    return stringify(s);
}

std::string
StringifyFormatter::format(const UseFlagName & s, const format::Masked & k) const
{
    if (_imp->f_use)
        return _imp->f_use->format(s, k);
    return stringify(s);
}

std::string
StringifyFormatter::format(const UseFlagName & s, const format::Plain & k) const
{
    if (_imp->f_use)
        return _imp->f_use->format(s, k);
    return stringify(s);
}

std::string
StringifyFormatter::format(const IUseFlag & s, const format::Enabled & k) const
{
    if (_imp->f_iuse)
        return _imp->f_iuse->format(s, k);
    return stringify(s);
}

std::string
StringifyFormatter::format(const IUseFlag & s, const format::Disabled & k) const
{
    if (_imp->f_iuse)
        return _imp->f_iuse->format(s, k);
    return stringify(s);
}

std::string
StringifyFormatter::format(const IUseFlag & s, const format::Forced & k) const
{
    if (_imp->f_iuse)
        return _imp->f_iuse->format(s, k);
    return stringify(s);
}

std::string
StringifyFormatter::format(const IUseFlag & s, const format::Masked & k) const
{
    if (_imp->f_iuse)
        return _imp->f_iuse->format(s, k);
    return stringify(s);
}

std::string
StringifyFormatter::decorate(const IUseFlag & f, const std::string & s, const format::Changed & k) const
{
    if (_imp->f_iuse)
        return _imp->f_iuse->decorate(f, s, k);
    return s;
}

std::string
StringifyFormatter::decorate(const IUseFlag & f, const std::string & s, const format::Added & k) const
{
    if (_imp->f_iuse)
        return _imp->f_iuse->decorate(f, s, k);
    return s;
}

std::string
StringifyFormatter::format(const IUseFlag & s, const format::Plain & k) const
{
    if (_imp->f_iuse)
        return _imp->f_iuse->format(s, k);
    return stringify(s);
}

std::string
StringifyFormatter::format(const KeywordName & s, const format::Accepted & k) const
{
    if (_imp->f_keyword)
        return _imp->f_keyword->format(s, k);
    return stringify(s);
}

std::string
StringifyFormatter::format(const KeywordName & s, const format::Unaccepted & k) const
{
    if (_imp->f_keyword)
        return _imp->f_keyword->format(s, k);
    return stringify(s);
}

std::string
StringifyFormatter::format(const KeywordName & s, const format::Plain & k) const
{
    if (_imp->f_keyword)
        return _imp->f_keyword->format(s, k);
    return stringify(s);
}

std::string
StringifyFormatter::format(const PackageDepSpec & s, const format::Plain & k) const
{
    if (_imp->f_package)
        return _imp->f_package->format(s, k);
    return stringify(s);
}

std::string
StringifyFormatter::format(const PackageDepSpec & s, const format::Installed & k) const
{
    if (_imp->f_package)
        return _imp->f_package->format(s, k);
    return stringify(s);
}

std::string
StringifyFormatter::format(const PackageDepSpec & s, const format::Installable & k) const
{
    if (_imp->f_package)
        return _imp->f_package->format(s, k);
    return stringify(s);
}

std::string
StringifyFormatter::format(const BlockDepSpec & s, const format::Plain & k) const
{
    if (_imp->f_block)
        return _imp->f_block->format(s, k);
    return stringify(s);
}

std::string
StringifyFormatter::format(const SimpleURIDepSpec & s, const format::Plain & k) const
{
    if (_imp->f_s_uri)
        return _imp->f_s_uri->format(s, k);
    return stringify(s);
}

std::string
StringifyFormatter::format(const FetchableURIDepSpec & s, const format::Plain & k) const
{
    if (_imp->f_f_uri)
        return _imp->f_f_uri->format(s, k);
    return stringify(s);
}

std::string
StringifyFormatter::format(const LicenseDepSpec & s, const format::Plain & k) const
{
    if (_imp->f_license)
        return _imp->f_license->format(s, k);
    return stringify(s);
}

std::string
StringifyFormatter::format(const LicenseDepSpec & s, const format::Accepted & k) const
{
    if (_imp->f_license)
        return _imp->f_license->format(s, k);
    return stringify(s);
}

std::string
StringifyFormatter::format(const LicenseDepSpec & s, const format::Unaccepted & k) const
{
    if (_imp->f_license)
        return _imp->f_license->format(s, k);
    return stringify(s);
}

std::string
StringifyFormatter::format(const DependencyLabelsDepSpec & s, const format::Plain & k) const
{
    if (_imp->f_dep_label)
        return _imp->f_dep_label->format(s, k);
    return stringify(s);
}

std::string
StringifyFormatter::format(const URILabelsDepSpec & s, const format::Plain & k) const
{
    if (_imp->f_uri_label)
        return _imp->f_uri_label->format(s, k);
    return stringify(s);
}

std::string
StringifyFormatter::format(const PlainTextDepSpec & s, const format::Plain & k) const
{
    if (_imp->f_plain)
        return _imp->f_plain->format(s, k);
    return stringify(s);
}

std::string
StringifyFormatter::format(const UseDepSpec & s, const format::Enabled & k) const
{
    if (_imp->f_use_dep)
        return _imp->f_use_dep->format(s, k);
    return stringify(s);
}

std::string
StringifyFormatter::format(const UseDepSpec & s, const format::Disabled & k) const
{
    if (_imp->f_use_dep)
        return _imp->f_use_dep->format(s, k);
    return stringify(s);
}

std::string
StringifyFormatter::format(const UseDepSpec & s, const format::Forced & k) const
{
    if (_imp->f_use_dep)
        return _imp->f_use_dep->format(s, k);
    return stringify(s);
}

std::string
StringifyFormatter::format(const UseDepSpec & s, const format::Masked & k) const
{
    if (_imp->f_use_dep)
        return _imp->f_use_dep->format(s, k);
    return stringify(s);
}

std::string
StringifyFormatter::format(const UseDepSpec & s, const format::Plain & k) const
{
    if (_imp->f_use_dep)
        return _imp->f_use_dep->format(s, k);
    return stringify(s);
}

std::string
StringifyFormatter::format(const NamedSetDepSpec & s, const format::Plain & k) const
{
    if (_imp->f_named)
        return _imp->f_named->format(s, k);
    return stringify(s);
}

std::string
StringifyFormatter::newline() const
{
    if (_imp->f_space)
        return _imp->f_space->newline();
    return "\n";
}

std::string
StringifyFormatter::indent(const int i) const
{
    if (_imp->f_space)
        return _imp->f_space->indent(i);
    return std::string(4 * i, ' ');
}

