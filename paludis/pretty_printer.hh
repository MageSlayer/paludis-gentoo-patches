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

#ifndef PALUDIS_GUARD_PALUDIS_PRETTY_PRINTER_HH
#define PALUDIS_GUARD_PALUDIS_PRETTY_PRINTER_HH 1

#include <paludis/pretty_printer-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/fs_path-fwd.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/name-fwd.hh>
#include <paludis/maintainer-fwd.hh>
#include <string>
#include <memory>

namespace paludis
{
    class PALUDIS_VISIBLE PrettyPrinter
    {
        public:
            PrettyPrinter();
            virtual ~PrettyPrinter() = 0;

            PrettyPrinter(const PrettyPrinter &) = delete;
            PrettyPrinter & operator= (const PrettyPrinter &) = delete;

            virtual const std::string indentify(const int) const = 0;
            virtual const std::string newline() const = 0;

            virtual const std::string prettify(const PackageDepSpec &) const = 0;
            virtual const std::string prettify(const BlockDepSpec &) const = 0;
            virtual const std::string prettify(const ConditionalDepSpec &) const = 0;
            virtual const std::string prettify(const NamedSetDepSpec &) const = 0;
            virtual const std::string prettify(const SimpleURIDepSpec &) const = 0;
            virtual const std::string prettify(const PlainTextDepSpec &) const = 0;
            virtual const std::string prettify(const LicenseDepSpec &) const = 0;
            virtual const std::string prettify(const FetchableURIDepSpec &) const = 0;
            virtual const std::string prettify(const URILabelsDepSpec &) const = 0;
            virtual const std::string prettify(const DependenciesLabelsDepSpec &) const = 0;
            virtual const std::string prettify(const PlainTextLabelDepSpec &) const = 0;

            virtual const std::string prettify(const std::shared_ptr<const PackageID> &) const = 0;

            virtual const std::string prettify(const bool) const = 0;

            virtual const std::string prettify(const long) const = 0;

            virtual const std::string prettify(const std::string &) const = 0;

            virtual const std::string prettify(const std::pair<const std::string, std::string> &) const = 0;

            virtual const std::string prettify(const FSPath &) const = 0;

            virtual const std::string prettify(const KeywordName &) const = 0;

            virtual const std::string prettify(const Maintainer &) const = 0;
    };
}

#endif
