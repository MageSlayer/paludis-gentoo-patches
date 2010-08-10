/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_SRC_CLIENTS_CAVE_EXCEPTIONS_HH
#define PALUDIS_GUARD_SRC_CLIENTS_CAVE_EXCEPTIONS_HH 1

#include <paludis/util/exception.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/name-fwd.hh>
#include <paludis/filter-fwd.hh>
#include <paludis/environment-fwd.hh>
#include <memory>

namespace paludis
{
    namespace cave
    {
        class PALUDIS_VISIBLE NothingMatching :
            public Exception
        {
            public:
                NothingMatching(const PackageDepSpec &) throw ();
                NothingMatching(const std::string &) throw ();
        };

        class PALUDIS_VISIBLE BeMoreSpecific :
            public Exception
        {
            public:
                BeMoreSpecific(const PackageDepSpec &, const std::shared_ptr<const PackageIDSequence> &) throw ();
        };

        class PALUDIS_VISIBLE BadIDForCommand :
            public Exception
        {
            public:
                BadIDForCommand(
                        const PackageDepSpec &,
                        const std::shared_ptr<const PackageID> &,
                        const std::string & r) throw ();
        };

        class PALUDIS_VISIBLE BadRepositoryForCommand :
            public Exception
        {
            public:
                BadRepositoryForCommand(
                        const RepositoryName &,
                        const std::string & r) throw ();
        };

        void nothing_matching_error(
                const Environment * const,
                const std::string &,
                const Filter &) PALUDIS_ATTRIBUTE((noreturn));
    }
}

#endif
