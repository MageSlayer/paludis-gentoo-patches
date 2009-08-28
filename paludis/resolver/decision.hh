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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_DECISION_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_DECISION_HH 1

#include <paludis/resolver/decision-fwd.hh>
#include <paludis/util/named_value.hh>
#include <paludis/package_id.hh>

namespace paludis
{
    namespace n
    {
        struct if_package_id;
        struct is_best;
        struct is_installed;
        struct is_nothing;
        struct is_same;
        struct is_same_version;
        struct is_transient;
    }

    namespace resolver
    {
        struct Decision
        {
            NamedValue<n::if_package_id, std::tr1::shared_ptr<const PackageID> > if_package_id;
            NamedValue<n::is_best, bool> is_best;
            NamedValue<n::is_installed, bool> is_installed;
            NamedValue<n::is_nothing, bool> is_nothing;
            NamedValue<n::is_same, bool> is_same;
            NamedValue<n::is_same_version, bool> is_same_version;
            NamedValue<n::is_transient, bool> is_transient;
        };
    }
}

#endif
