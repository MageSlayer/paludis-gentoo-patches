/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_SRC_OUTPUT_COLOUR_FORMATTER_HH
#define PALUDIS_GUARD_SRC_OUTPUT_COLOUR_FORMATTER_HH 1

#include <paludis/formatter.hh>
#include <paludis/name-fwd.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/util/fs_path-fwd.hh>

class ColourFormatter :
    public paludis::CanFormat<paludis::ChoiceValue>,
    public paludis::CanFormat<paludis::KeywordName>,
    public paludis::CanFormat<paludis::ConditionalDepSpec>,
    public paludis::CanFormat<paludis::PackageDepSpec>,
    public paludis::CanFormat<paludis::BlockDepSpec>,
    public paludis::CanFormat<paludis::DependenciesLabelsDepSpec>,
    public paludis::CanFormat<paludis::URILabelsDepSpec>,
    public paludis::CanFormat<paludis::PlainTextDepSpec>,
    public paludis::CanFormat<paludis::SimpleURIDepSpec>,
    public paludis::CanFormat<paludis::PlainTextLabelDepSpec>,
    public paludis::CanFormat<paludis::FetchableURIDepSpec>,
    public paludis::CanFormat<paludis::LicenseDepSpec>,
    public paludis::CanFormat<paludis::NamedSetDepSpec>,
    public paludis::CanFormat<paludis::FSPath>,
    public paludis::CanFormat<paludis::PackageID>,
    public paludis::CanFormat<std::string>,
    public paludis::CanSpace
{
    public:
        std::string format(const paludis::ChoiceValue &, const paludis::format::Plain &) const;
        std::string format(const paludis::ChoiceValue &, const paludis::format::Enabled &) const;
        std::string format(const paludis::ChoiceValue &, const paludis::format::Disabled &) const;
        std::string format(const paludis::ChoiceValue &, const paludis::format::Forced &) const;
        std::string format(const paludis::ChoiceValue &, const paludis::format::Masked &) const;
        std::string decorate(const paludis::ChoiceValue &, const std::string &, const paludis::format::Added &) const;
        std::string decorate(const paludis::ChoiceValue &, const std::string &, const paludis::format::Changed &) const;

        std::string format(const paludis::ConditionalDepSpec &, const paludis::format::Plain &) const;
        std::string format(const paludis::ConditionalDepSpec &, const paludis::format::Enabled &) const;
        std::string format(const paludis::ConditionalDepSpec &, const paludis::format::Disabled &) const;
        std::string format(const paludis::ConditionalDepSpec &, const paludis::format::Forced &) const;
        std::string format(const paludis::ConditionalDepSpec &, const paludis::format::Masked &) const;
        std::string decorate(const paludis::ConditionalDepSpec &, const std::string &, const paludis::format::Added &) const;
        std::string decorate(const paludis::ConditionalDepSpec &, const std::string &, const paludis::format::Changed &) const;

        std::string format(const paludis::PackageDepSpec &, const paludis::format::Plain &) const;
        std::string format(const paludis::PackageDepSpec &, const paludis::format::Installed &) const;
        std::string format(const paludis::PackageDepSpec &, const paludis::format::Installable &) const;

        std::string format(const paludis::PlainTextDepSpec &, const paludis::format::Plain &) const;

        std::string format(const paludis::LicenseDepSpec &, const paludis::format::Plain &) const;
        std::string format(const paludis::LicenseDepSpec &, const paludis::format::Accepted &) const;
        std::string format(const paludis::LicenseDepSpec &, const paludis::format::Unaccepted &) const;

        std::string format(const paludis::KeywordName &, const paludis::format::Plain &) const;
        std::string format(const paludis::KeywordName &, const paludis::format::Accepted &) const;
        std::string format(const paludis::KeywordName &, const paludis::format::Unaccepted &) const;

        std::string format(const std::string &, const paludis::format::Plain &) const;

        std::string format(const paludis::PlainTextLabelDepSpec &, const paludis::format::Plain &) const;

        std::string format(const paludis::URILabelsDepSpec &, const paludis::format::Plain &) const;

        std::string format(const paludis::DependenciesLabelsDepSpec &, const paludis::format::Plain &) const;

        std::string format(const paludis::FetchableURIDepSpec &, const paludis::format::Plain &) const;

        std::string format(const paludis::SimpleURIDepSpec &, const paludis::format::Plain &) const;

        std::string format(const paludis::NamedSetDepSpec &, const paludis::format::Plain &) const;

        std::string format(const paludis::BlockDepSpec &, const paludis::format::Plain &) const;

        std::string format(const paludis::FSPath &, const paludis::format::Plain &) const;

        std::string format(const paludis::PackageID &, const paludis::format::Plain &) const;
        std::string format(const paludis::PackageID &, const paludis::format::Installed &) const;
        std::string format(const paludis::PackageID &, const paludis::format::Installable &) const;

        std::string newline() const;
        std::string indent(const int) const;
};

#endif
