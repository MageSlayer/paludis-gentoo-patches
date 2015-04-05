/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2014 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_DESTINATION_UTILS_FWD_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_DESTINATION_UTILS_FWD_HH 1

#include <paludis/util/attributes.hh>
#include <paludis/resolver/destination_types-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/generator-fwd.hh>
#include <paludis/filtered_generator-fwd.hh>
#include <paludis/filter-fwd.hh>
#include <paludis/environment-fwd.hh>

namespace paludis
{
    namespace resolver
    {
        bool can_make_binary_for(const std::shared_ptr<const PackageID> & id) PALUDIS_ATTRIBUTE((warn_unused_result)) PALUDIS_VISIBLE;
        bool is_already_binary(const std::shared_ptr<const PackageID> & id) PALUDIS_ATTRIBUTE((warn_unused_result)) PALUDIS_VISIBLE;
        bool can_chroot(const std::shared_ptr<const PackageID> & id) PALUDIS_ATTRIBUTE((warn_unused_result)) PALUDIS_VISIBLE;
        bool can_cross_compile(const std::shared_ptr<const PackageID> & id)
            PALUDIS_ATTRIBUTE((warn_unused_result)) PALUDIS_VISIBLE;

        FilteredGenerator destination_filtered_generator(
                const Environment * const,
                const DestinationType,
                const Generator &) PALUDIS_ATTRIBUTE((warn_unused_result)) PALUDIS_VISIBLE;

        Filter make_destination_type_filter(
                const DestinationType) PALUDIS_ATTRIBUTE((warn_unused_result)) PALUDIS_VISIBLE;
    }
}

#endif
