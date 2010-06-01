/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_SRC_CLIENTS_CAVE_COLOUR_FORMATTER_HH
#define PALUDIS_GUARD_SRC_CLIENTS_CAVE_COLOUR_FORMATTER_HH 1

#include <paludis/util/attributes.hh>
#include <paludis/util/fs_entry-fwd.hh>
#include <paludis/formatter.hh>

namespace paludis
{
    namespace cave
    {
        class PALUDIS_VISIBLE ColourFormatter :
            public CanFormat<KeywordName>,
            public CanFormat<std::string>,
            public CanFormat<PackageID>,
            public CanFormat<LicenseDepSpec>,
            public CanFormat<ConditionalDepSpec>,
            public CanFormat<PlainTextDepSpec>,
            public CanFormat<SimpleURIDepSpec>,
            public CanFormat<FetchableURIDepSpec>,
            public CanFormat<URILabelsDepSpec>,
            public CanFormat<PackageDepSpec>,
            public CanFormat<DependenciesLabelsDepSpec>,
            public CanFormat<BlockDepSpec>,
            public CanFormat<NamedSetDepSpec>,
            public CanFormat<PlainTextLabelDepSpec>,
            public CanFormat<ChoiceValue>,
            public CanFormat<FSEntry>,
            public CanSpace
        {
            private:
                const int _initial_indent;

            public:
                ColourFormatter(const int initial_indent);

                std::string format(const ChoiceValue &, const format::Plain &) const;
                std::string format(const ChoiceValue &, const format::Enabled &) const;
                std::string format(const ChoiceValue &, const format::Disabled &) const;
                std::string format(const ChoiceValue &, const format::Forced &) const;
                std::string format(const ChoiceValue &, const format::Masked &) const;
                std::string decorate(const ChoiceValue &, const std::string &, const format::Added &) const;
                std::string decorate(const ChoiceValue &, const std::string &, const format::Changed &) const;

                std::string format(const KeywordName &, const format::Plain &) const;
                std::string format(const KeywordName &, const format::Accepted &) const;
                std::string format(const KeywordName &, const format::Unaccepted &) const;

                std::string format(const std::string &, const format::Plain &) const;

                std::string format(const PackageID &, const format::Plain &) const;
                std::string format(const PackageID &, const format::Installed &) const;
                std::string format(const PackageID &, const format::Installable &) const;

                std::string format(const LicenseDepSpec &, const format::Plain &) const;
                std::string format(const LicenseDepSpec &, const format::Accepted &) const;
                std::string format(const LicenseDepSpec &, const format::Unaccepted &) const;

                std::string format(const ConditionalDepSpec &, const format::Plain &) const;
                std::string format(const ConditionalDepSpec &, const format::Enabled &) const;
                std::string format(const ConditionalDepSpec &, const format::Disabled &) const;
                std::string format(const ConditionalDepSpec &, const format::Forced &) const;
                std::string format(const ConditionalDepSpec &, const format::Masked &) const;
                std::string decorate(const ConditionalDepSpec &, const std::string &, const format::Added &) const;
                std::string decorate(const ConditionalDepSpec &, const std::string &, const format::Changed &) const;

                std::string format(const PlainTextDepSpec &, const format::Plain &) const;

                std::string format(const SimpleURIDepSpec &, const format::Plain &) const;

                std::string format(const FetchableURIDepSpec &, const format::Plain &) const;

                std::string format(const URILabelsDepSpec &, const format::Plain &) const;

                std::string format(const PlainTextLabelDepSpec &, const format::Plain &) const;

                std::string format(const PackageDepSpec &, const format::Plain &) const;
                std::string format(const PackageDepSpec &, const format::Installed &) const;
                std::string format(const PackageDepSpec &, const format::Installable &) const;

                std::string format(const DependenciesLabelsDepSpec &, const format::Plain &) const;

                std::string format(const BlockDepSpec &, const format::Plain &) const;

                std::string format(const NamedSetDepSpec &, const format::Plain &) const;

                std::string format(const FSEntry &, const format::Plain &) const;

                std::string newline() const;
                std::string indent(const int) const;
        };
    }
}

#endif
