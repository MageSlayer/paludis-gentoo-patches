/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_E_DEP_PARSER_HH
#define PALUDIS_GUARD_PALUDIS_E_DEP_PARSER_HH 1

#include <paludis/repositories/e/dep_parser-fwd.hh>
#include <paludis/spec_tree.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/repositories/e/eapi-fwd.hh>
#include <paludis/util/exception.hh>
#include <paludis/environment-fwd.hh>
#include <string>

/** \file
 * Declarations for the DepParser routines.
 *
 * \ingroup grpdepparser
 */

namespace paludis
{
    namespace erepository
    {
        class PALUDIS_VISIBLE EDepParseError :
            public Exception
        {
            public:
                /**
                 * Constructor.
                 */
                EDepParseError(const std::string & dep_string,
                        const std::string & message) throw ();
        };

        /**
         * Parse a dependency heirarchy.
         */
        std::shared_ptr<DependencySpecTree> parse_depend(const std::string & s,
                const Environment * const, const std::shared_ptr<const PackageID> &, const EAPI &) PALUDIS_VISIBLE;

        /**
         * Parse a dep spec label.
         */
        std::shared_ptr<DependenciesLabelsDepSpec> parse_dependency_label(
                const Environment * const,
                const std::shared_ptr<const PackageID> &,
                const std::string & s,
                const EAPI &) PALUDIS_VISIBLE;

        /**
         * Parse a provide heirarchy.
         */
        std::shared_ptr<ProvideSpecTree> parse_provide(const std::string & s,
                const Environment * const, const std::shared_ptr<const PackageID> &, const EAPI &) PALUDIS_VISIBLE;

        /**
         * Parse a restrict or plain text heirarchy.
         */
        std::shared_ptr<PlainTextSpecTree> parse_plain_text(const std::string & s,
                const Environment * const, const std::shared_ptr<const PackageID> &, const EAPI &) PALUDIS_VISIBLE;

        /**
         * Parse a myoptions heirarchy.
         */
        std::shared_ptr<PlainTextSpecTree> parse_myoptions(const std::string & s,
                const Environment * const, const std::shared_ptr<const PackageID> &, const EAPI &) PALUDIS_VISIBLE;

        /**
         * Parse a required_use heirarchy.
         */
        std::shared_ptr<RequiredUseSpecTree> parse_required_use(const std::string & s,
                const Environment * const, const std::shared_ptr<const PackageID> &, const EAPI &) PALUDIS_VISIBLE;

        /**
         * Parse a fetchable uri heirarchy.
         */
        std::shared_ptr<FetchableURISpecTree> parse_fetchable_uri(const std::string & s,
                const Environment * const, const std::shared_ptr<const PackageID> &, const EAPI &) PALUDIS_VISIBLE;

        /**
         * Parse a simple uri heirarchy.
         */
        std::shared_ptr<SimpleURISpecTree> parse_simple_uri(const std::string & s,
                const Environment * const, const std::shared_ptr<const PackageID> &, const EAPI &) PALUDIS_VISIBLE;

        /**
         * Parse a license heirarchy.
         */
        std::shared_ptr<LicenseSpecTree> parse_license(const std::string & s,
                const Environment * const, const std::shared_ptr<const PackageID> &, const EAPI &) PALUDIS_VISIBLE;

        /**
         * Parse a URI label.
         */
        std::shared_ptr<URILabelsDepSpec> parse_uri_label(const std::string & s, const EAPI &) PALUDIS_VISIBLE;

        /**
         * Parse a plain text label.
         */
        std::shared_ptr<PlainTextLabelDepSpec> parse_plain_text_label(const std::string & s) PALUDIS_VISIBLE;
    }
}

#endif
