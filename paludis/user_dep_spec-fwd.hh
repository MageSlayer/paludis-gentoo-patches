/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_USER_DEP_SPEC_FWD_HH
#define PALUDIS_GUARD_PALUDIS_USER_DEP_SPEC_FWD_HH 1

#include <paludis/util/attributes.hh>
#include <paludis/util/options-fwd.hh>
#include <paludis/util/exception.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/version_spec-fwd.hh>
#include <memory>
#include <iosfwd>

namespace paludis
{

#include <paludis/user_dep_spec-se.hh>

    /**
     * Options for parse_user_package_dep_spec.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     */
    typedef Options<UserPackageDepSpecOption> UserPackageDepSpecOptions;

    /**
     * Thrown by paludis::parse_user_package_dep_spec if options includes
     * updso_throw_if_set and we're given a set.
     *
     * \since 0.28
     * \ingroup g_dep_spec
     * \ingroup g_exceptions
     */
    class PALUDIS_VISIBLE GotASetNotAPackageDepSpec :
        public Exception
    {
        public:
            ///\name Basic Operations
            ///\{

            GotASetNotAPackageDepSpec(const std::string &) throw ();

            ///\}
    };

    /**
     * The VersionSpecOptions to use for parsing a user spec.
     *
     * \since 0.38
     * \ingroup g_dep_spec
     */
    VersionSpecOptions user_version_spec_options() PALUDIS_ATTRIBUTE((warn_unused_result)) PALUDIS_VISIBLE;
}

#endif
