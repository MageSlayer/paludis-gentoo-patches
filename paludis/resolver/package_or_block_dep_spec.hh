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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_PACKAGE_OR_BLOCK_DEP_SPEC_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_PACKAGE_OR_BLOCK_DEP_SPEC_HH 1

#include <paludis/resolver/package_or_block_dep_spec-fwd.hh>
#include <paludis/util/named_value.hh>
#include <paludis/dep_spec.hh>
#include <paludis/serialise-fwd.hh>

namespace paludis
{
    namespace n
    {
        typedef Name<struct name_if_package> if_package;
        typedef Name<struct name_if_block> if_block;
    }

    namespace resolver
    {
        struct PackageOrBlockDepSpec
        {
            NamedValue<n::if_block, std::shared_ptr<BlockDepSpec> > if_block;
            NamedValue<n::if_package, std::shared_ptr<PackageDepSpec> > if_package;

            PackageOrBlockDepSpec(const BlockDepSpec &);
            PackageOrBlockDepSpec(const PackageDepSpec &);

            void serialise(Serialiser &) const;

            static PackageOrBlockDepSpec deserialise(
                    Deserialisation & d,
                    const std::shared_ptr<const PackageID> & for_id) PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }
}

#endif
