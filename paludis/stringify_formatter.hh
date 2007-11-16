/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh
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
#include <paludis/util/fs_entry-fwd.hh>

/** \file
 * Declarations for the StringifyFormatter class.
 *
 * \ingroup g_formatters
 *
 * \section Examples
 *
 * - \ref example_stringify_formatter.cc "example_stringify_formatter.cc"
 * - \ref example_formatter.cc "example_formatter.cc"
 */

namespace paludis
{
    /**
     * A StringifyFormatter is a Formatter that implements every format function
     * by calling paludis::stringify().
     *
     * A StringifyFormatter can also act as a wrapper class around another
     * Formatter. Any CanFormat<> interface implemented by that other formatter
     * is used; any not implemented by the other formatter is implemented using
     * paludis::stringify().
     *
     * Indenting is done via simple spaces; newlines are done via a newline
     * character. Again, when used as a wrapper, this can be overridden by the
     * wrapped class.
     *
     * \ingroup g_formatters
     * \since 0.26
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE StringifyFormatter :
        private PrivateImplementationPattern<StringifyFormatter>,
        public CanFormat<std::string>,
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
        public CanFormat<NamedSetDepSpec>,
        public CanFormat<FSEntry>,
        public CanFormat<PackageID>,
        public CanSpace
    {
        private:
            StringifyFormatter(const StringifyFormatter &);

        public:
            ///\name Basic operations
            ///\{

            StringifyFormatter();

            /**
             * StringifyFormatter can be constructed as a wrapper around another
             * formatter.
             */
            template <typename T_> explicit StringifyFormatter(const T_ &);

            ~StringifyFormatter();

            ///\}

            virtual std::string format(const std::string &, const format::Plain &) const;

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

            virtual std::string format(const NamedSetDepSpec &, const format::Plain &) const;

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

            virtual std::string format(const FSEntry &, const format::Plain &) const;

            virtual std::string format(const PackageID &, const format::Plain &) const;
            virtual std::string format(const PackageID &, const format::Installed &) const;
            virtual std::string format(const PackageID &, const format::Installable &) const;

            virtual std::string newline() const;
            virtual std::string indent(const int) const;
    };
}

#endif
