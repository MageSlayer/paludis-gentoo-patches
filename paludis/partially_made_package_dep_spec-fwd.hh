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
#ifndef PALUDIS_GUARD_PALUDIS_PARTIALLY_MADE_PACKAGE_DEP_SPEC_FWD_HH
#define PALUDIS_GUARD_PALUDIS_PARTIALLY_MADE_PACKAGE_DEP_SPEC_FWD_HH 1

#include <paludis/util/attributes.hh>
#include <paludis/util/options-fwd.hh>
#include <iosfwd>

namespace paludis
{

#include <paludis/partially_made_package_dep_spec-se.hh>

    /**
     * Options for PartiallyMadePackageDepSpec.
     *
     * \ingroup g_dep_spec
     * \since 0.38
     */
    typedef Options<PartiallyMadePackageDepSpecOption> PartiallyMadePackageDepSpecOptions;

    class PartiallyMadePackageDepSpec;

    /**
     * Create a PackageDepSpec from various rules.
     *
     * Note the return type is a PartiallyMadePackageDepSpec, which is implicitly convertible to
     * a PackageDepSpec.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     */
    PartiallyMadePackageDepSpec make_package_dep_spec(const PartiallyMadePackageDepSpecOptions &) PALUDIS_VISIBLE;
}

#endif
