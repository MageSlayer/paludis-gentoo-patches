/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_STRINGIFY_FORMATTER_HH
#define PALUDIS_GUARD_PALUDIS_STRINGIFY_FORMATTER_HH 1

#include <paludis/stringify_formatter-fwd.hh>
#include <paludis/formatter.hh>
#include <paludis/name.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/util/private_implementation_pattern.hh>

namespace paludis
{
    class PALUDIS_VISIBLE StringifyFormatter :
        private PrivateImplementationPattern<StringifyFormatter>,
        public CanFormat<UseFlagName>,
        public CanFormat<IUseFlag>,
        public CanFormat<KeywordName>,
        public CanFormat<PackageDepSpec>,
        public CanFormat<BlockDepSpec>,
        public CanFormat<FetchableURIDepSpec>,
        public CanFormat<SimpleURIDepSpec>,
        public CanFormat<DependencyLabelsDepSpec>,
        public CanFormat<URILabelsDepSpec>,
        public CanFormat<PlainTextDepSpec>,
        public CanFormat<LicenseDepSpec>,
        public CanFormat<UseDepSpec>,
        public CanSpace
    {
        private:
            StringifyFormatter(const StringifyFormatter &);

        public:
            StringifyFormatter();

            template <typename T_> StringifyFormatter(const T_ &);

            ~StringifyFormatter();

            virtual std::string format(const UseFlagName &, const format::Enabled &) const;
            virtual std::string format(const UseFlagName &, const format::Disabled &) const;
            virtual std::string format(const UseFlagName &, const format::Forced &) const;
            virtual std::string format(const UseFlagName &, const format::Masked &) const;
            virtual std::string format(const UseFlagName &, const format::Plain &) const;

            virtual std::string format(const IUseFlag &, const format::Enabled &) const;
            virtual std::string format(const IUseFlag &, const format::Disabled &) const;
            virtual std::string format(const IUseFlag &, const format::Forced &) const;
            virtual std::string format(const IUseFlag &, const format::Masked &) const;
            virtual std::string format(const IUseFlag &, const format::Plain &) const;
            virtual std::string decorate(const IUseFlag &, const std::string &, const format::Changed &) const;
            virtual std::string decorate(const IUseFlag &, const std::string &, const format::Added &) const;

            virtual std::string format(const KeywordName &, const format::Accepted &) const;
            virtual std::string format(const KeywordName &, const format::Unaccepted &) const;
            virtual std::string format(const KeywordName &, const format::Plain &) const;

            virtual std::string format(const PackageDepSpec &, const format::Plain &) const;
            virtual std::string format(const PackageDepSpec &, const format::Installed &) const;
            virtual std::string format(const PackageDepSpec &, const format::Installable &) const;

            virtual std::string format(const BlockDepSpec &, const format::Plain &) const;

            virtual std::string format(const FetchableURIDepSpec &, const format::Plain &) const;

            virtual std::string format(const SimpleURIDepSpec &, const format::Plain &) const;

            virtual std::string format(const DependencyLabelsDepSpec &, const format::Plain &) const;

            virtual std::string format(const URILabelsDepSpec &, const format::Plain &) const;

            virtual std::string format(const PlainTextDepSpec &, const format::Plain &) const;

            virtual std::string format(const LicenseDepSpec &, const format::Plain &) const;
            virtual std::string format(const LicenseDepSpec &, const format::Accepted &) const;
            virtual std::string format(const LicenseDepSpec &, const format::Unaccepted &) const;

            virtual std::string format(const UseDepSpec &, const format::Enabled &) const;
            virtual std::string format(const UseDepSpec &, const format::Disabled &) const;
            virtual std::string format(const UseDepSpec &, const format::Forced &) const;
            virtual std::string format(const UseDepSpec &, const format::Masked &) const;
            virtual std::string format(const UseDepSpec &, const format::Plain &) const;

            virtual std::string newline() const;
            virtual std::string indent(const int) const;
    };
}

#endif
