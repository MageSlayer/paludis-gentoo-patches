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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_QPN_S_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_QPN_S_HH 1

#include <paludis/resolver/qpn_s-fwd.hh>
#include <paludis/util/named_value.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/name.hh>
#include <paludis/filter-fwd.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <tr1/memory>

namespace paludis
{
    namespace n
    {
        struct package;
        struct slot_name_or_null;
    }

    namespace resolver
    {
        class PALUDIS_VISIBLE QPN_S :
            private PrivateImplementationPattern<QPN_S>
        {
            public:
                QPN_S(const QualifiedPackageName &, const std::tr1::shared_ptr<const SlotName> &);
                QPN_S(const PackageDepSpec &, const std::tr1::shared_ptr<const SlotName> &);
                explicit QPN_S(const std::tr1::shared_ptr<const PackageID> &);
                QPN_S(const QPN_S &);
                ~QPN_S();

                const QualifiedPackageName package() const PALUDIS_ATTRIBUTE((warn_unused_result));
                const std::tr1::shared_ptr<const SlotName> slot_name_or_null() const PALUDIS_ATTRIBUTE((warn_unused_result));

                Filter make_slot_filter() const PALUDIS_ATTRIBUTE((warn_unused_result));

                bool operator< (const QPN_S & other) const PALUDIS_ATTRIBUTE((warn_unused_result));
                bool operator== (const QPN_S & other) const PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }

#ifdef PALUDIS_HAVE_EXTERN_TEMPLATE
    extern template class PrivateImplementationPattern<resolver::QPN_S>;
#endif
}

#endif
