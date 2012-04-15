/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_COLLECT_DEPPED_UPON_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_COLLECT_DEPPED_UPON_HH 1

#include <paludis/resolver/collect_depped_upon-fwd.hh>
#include <paludis/resolver/resolvent.hh>

#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/named_value.hh>

#include <paludis/package_id-fwd.hh>
#include <paludis/serialise-fwd.hh>

namespace paludis
{
    namespace n
    {
        typedef Name<struct name_active_dependency_labels_as_string> active_dependency_labels_as_string;
        typedef Name<struct name_package_id> package_id;
        typedef Name<struct name_resolvent> resolvent;
    }

    namespace resolver
    {
        struct DependentPackageID
        {
            NamedValue<n::active_dependency_labels_as_string, std::string> active_dependency_labels_as_string;
            NamedValue<n::package_id, std::shared_ptr<const PackageID> > package_id;
            NamedValue<n::resolvent, Resolvent> resolvent;

            void serialise(Serialiser &) const;
            static const DependentPackageID deserialise(Deserialisation &) PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }

    extern template class PALUDIS_VISIBLE Sequence<resolver::DependentPackageID>;
    extern template class PALUDIS_VISIBLE WrappedForwardIterator<Sequence<resolver::DependentPackageID>::ConstIteratorTag, const resolver::DependentPackageID>;
}

#endif
