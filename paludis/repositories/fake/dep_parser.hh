/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_FAKE_DEP_PARSER_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_FAKE_DEP_PARSER_HH 1

#include <paludis/util/exception.hh>
#include <paludis/spec_tree.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/environment-fwd.hh>
#include <string>
#include <functional>

namespace paludis
{
    namespace fakerepository
    {
        class PALUDIS_VISIBLE FakeDepParseError :
            public Exception
        {
            public:
                /**
                 * Constructor.
                 */
                FakeDepParseError(const std::string & dep_string,
                        const std::string & message) noexcept;
        };

        /**
         * Parse a dependency heirarchy.
         */
        std::shared_ptr<DependencySpecTree> parse_depend(const std::string & s,
                const Environment * const) PALUDIS_VISIBLE;

        /**
         * Parse a fetchable uri heirarchy.
         */
        std::shared_ptr<FetchableURISpecTree> parse_fetchable_uri(const std::string & s,
                const Environment * const) PALUDIS_VISIBLE;

        /**
         * Parse a simple uri heirarchy.
         */
        std::shared_ptr<SimpleURISpecTree> parse_simple_uri(const std::string & s,
                const Environment * const) PALUDIS_VISIBLE;

        /**
         * Parse a license heirarchy.
         */
        std::shared_ptr<LicenseSpecTree> parse_license(const std::string & s,
                const Environment * const) PALUDIS_VISIBLE;
    }
}

#endif
