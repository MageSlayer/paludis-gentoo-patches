/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_UNFORMATTED_PRETTY_PRINTER_HH
#define PALUDIS_GUARD_PALUDIS_UNFORMATTED_PRETTY_PRINTER_HH 1

#include <paludis/pretty_printer.hh>

namespace paludis
{
    class PALUDIS_VISIBLE UnformattedPrettyPrinter :
        public PrettyPrinter
    {
        public:
            UnformattedPrettyPrinter();

            virtual const std::string indentify(const int) const;
            virtual const std::string newline() const;

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
    };
}

#endif
