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

#ifndef PALUDIS_GUARD_PALUDIS_PACKAGE_DEP_SPEC_COLLECTION_HH
#define PALUDIS_GUARD_PALUDIS_PACKAGE_DEP_SPEC_COLLECTION_HH 1

#include <paludis/package_dep_spec_collection-fwd.hh>
#include <paludis/util/pimp.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/match_package-fwd.hh>
#include <memory>

namespace paludis
{
    class PALUDIS_VISIBLE PackageDepSpecCollection :
        private Pimp<PackageDepSpecCollection>
    {
        public:
            explicit PackageDepSpecCollection(const std::shared_ptr<const PackageID> & from_id);
            ~PackageDepSpecCollection();

            void insert(const PackageDepSpec &);

            bool match_any(
                    const Environment * const,
                    const std::shared_ptr<const PackageID> & id,
                    const MatchPackageOptions &) const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    extern template class Pimp<PackageDepSpecCollection>;
}

#endif
