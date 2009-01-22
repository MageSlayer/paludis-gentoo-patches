/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_PACKAGE_DEP_SPEC_PROPERTIES_FWD_HH
#define PALUDIS_GUARD_PALUDIS_PACKAGE_DEP_SPEC_PROPERTIES_FWD_HH 1

#include <paludis/dep_spec-fwd.hh>
#include <paludis/util/attributes.hh>

namespace paludis
{
    struct PackageDepSpecProperties;

    /**
     * Does a PackageDepSpec match the specified property requirements?
     *
     * We have a lot of code that wants to know things like "does this spec
     * consist exactly of a cat/pkg:slot?" and "does this spec have anything
     * other than a cat, pkg or slot?". All of these places need to be updated
     * every time a new PackageDepSpec attribute comes along. Using this
     * function makes sure the compiler catches any properties we haven't
     * updated.
     *
     * \ingroup g_dep_spec
     * \since 0.34.1
     */
    bool package_dep_spec_has_properties(const PackageDepSpec &, const PackageDepSpecProperties &)
        PALUDIS_VISIBLE PALUDIS_ATTRIBUTE((warn_unused_result));
}

#endif
