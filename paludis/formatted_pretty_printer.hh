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

#ifndef PALUDIS_GUARD_PALUDIS_FORMATTED_PRETTY_PRINTER_HH
#define PALUDIS_GUARD_PALUDIS_FORMATTED_PRETTY_PRINTER_HH 1

#include <paludis/formatted_pretty_printer-fwd.hh>
#include <paludis/util/pimp.hh>
#include <paludis/pretty_printer.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <memory>

namespace paludis
{
    class PALUDIS_VISIBLE FormattedPrettyPrinter :
        public PrettyPrinter
    {
        private:
            Pimp<FormattedPrettyPrinter> _imp;

        protected:
            virtual const std::string format_enabled(const std::string &) const = 0;
            virtual const std::string format_disabled(const std::string &) const = 0;
            virtual const std::string format_installed(const std::string &) const = 0;
            virtual const std::string format_installable(const std::string &) const = 0;
            virtual const std::string format_masked(const std::string &) const = 0;
            virtual const std::string format_plain(const std::string &) const = 0;

        public:
            FormattedPrettyPrinter(
                    const Environment * const,
                    const std::shared_ptr<const PackageID> &);
            ~FormattedPrettyPrinter();

            virtual const std::string prettify(const PackageDepSpec &) const;
            virtual const std::string prettify(const BlockDepSpec &) const;
            virtual const std::string prettify(const ConditionalDepSpec &) const;
            virtual const std::string prettify(const NamedSetDepSpec &) const;
            virtual const std::string prettify(const SimpleURIDepSpec &) const;
            virtual const std::string prettify(const PlainTextDepSpec &) const;
            virtual const std::string prettify(const LicenseDepSpec &) const;
            virtual const std::string prettify(const FetchableURIDepSpec &) const;
            virtual const std::string prettify(const URILabelsDepSpec &) const;
            virtual const std::string prettify(const DependenciesLabelsDepSpec &) const;
            virtual const std::string prettify(const PlainTextLabelDepSpec &) const;

            virtual const std::string prettify(const std::shared_ptr<const PackageID> &) const;

            virtual const std::string prettify(const bool) const;

            virtual const std::string prettify(const long) const;

            virtual const std::string prettify(const std::string &) const;

            virtual const std::string prettify(const std::pair<const std::string, std::string> &) const;

            virtual const std::string prettify(const FSPath &) const;

            virtual const std::string prettify(const KeywordName &) const;

            virtual const std::string prettify(const Maintainer &) const;
    };
}

#endif
