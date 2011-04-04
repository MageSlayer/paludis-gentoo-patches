/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_USER_DEP_SPEC_HH
#define PALUDIS_GUARD_PALUDIS_USER_DEP_SPEC_HH 1

#include <paludis/user_dep_spec-fwd.hh>
#include <paludis/dep_spec.hh>
#include <paludis/filter.hh>
#include <paludis/package_dep_spec_constraint-fwd.hh>

#include <paludis/util/pimp.hh>

#include <tuple>

namespace paludis
{
    /**
     * Create a PackageDepSpec from user input.
     *
     * \ingroup g_dep_spec
     * \since 0.28
     */
    PackageDepSpec parse_user_package_dep_spec(
            const std::string &,
            const Environment * const,
            const UserPackageDepSpecOptions &,
            const Filter & = filter::All()) PALUDIS_VISIBLE;

    /**
     * Create a PackageDepSpec from user input, restricted to not having
     * an Environment available.
     *
     * For use in test cases; should not be used elsewhere.
     *
     * \ingroup g_dep_spec
     * \since 0.61
     */
    PackageDepSpec envless_parse_package_dep_spec_for_tests(
            const std::string &) PALUDIS_VISIBLE;

    /**
     * Split up a [.key=value] into its component parts.
     *
     * \ingroup g_dep_spec
     * \since 0.61
     */
    std::tuple<KeyConstraintKeyType, std::string, KeyConstraintOperation, std::string> parse_user_key_constraint(
            const std::string &) PALUDIS_VISIBLE PALUDIS_ATTRIBUTE((warn_unused_result));
}

#endif
