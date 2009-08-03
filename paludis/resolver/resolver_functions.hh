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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_RESOLVER_FUNCTIONS_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_RESOLVER_FUNCTIONS_HH 1

#include <paludis/resolver/resolver_functions-fwd.hh>
#include <paludis/resolver/use_installed-fwd.hh>
#include <paludis/resolver/resolution-fwd.hh>
#include <paludis/resolver/qpn_s-fwd.hh>
#include <paludis/resolver/reason-fwd.hh>
#include <paludis/util/named_value.hh>
#include <tr1/functional>

namespace paludis
{
    namespace n
    {
        struct get_use_installed_fn;
    }

    namespace resolver
    {
        typedef std::tr1::function<UseInstalled (
                const QPN_S &,
                const PackageDepSpec &,
                const std::tr1::shared_ptr<const Reason> &)> GetUseInstalledFunction;

        struct ResolverFunctions
        {
            NamedValue<n::get_use_installed_fn, GetUseInstalledFunction> get_use_installed_fn;
        };
    }
}

#endif
