/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_PACKAGE_ID_FWD_HH
#define PALUDIS_GUARD_PALUDIS_PACKAGE_ID_FWD_HH 1

#include <iosfwd>
#include <paludis/util/attributes.hh>
#include <paludis/util/set-fwd.hh>
#include <paludis/util/sequence-fwd.hh>
#include <memory>

/** \file
 * Forward declarations for paludis/package_id.hh .
 *
 * \ingroup g_package_id
 */

namespace paludis
{
    class PackageID;
    class PackageIDSetComparator;
    class PackageIDComparator;

    /**
     * A PackageIDSequence holds a collection of PackageID instances that may
     * or may not have been ordered in a meaningful way.
     *
     * \ingroup g_package_id
     * \since 0.26
     */
    typedef Sequence<std::shared_ptr<const PackageID> > PackageIDSequence;

    /**
     * A PackageIDSet holds a collection of PackageID instances that have no
     * meaningful ordering.
     *
     * \ingroup g_package_id
     * \since 0.26
     */
    typedef Set<std::shared_ptr<const PackageID>, PackageIDSetComparator> PackageIDSet;

#include <paludis/package_id-se.hh>

    /**
     * A PackageID can be written to a stream.
     *
     * \ingroup g_package_id
     * \since 0.26
     */
    std::ostream & operator<< (std::ostream &, const PackageID &) PALUDIS_VISIBLE;
}

#endif
