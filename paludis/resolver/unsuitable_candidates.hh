/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_UNSUITABLE_CANDIDATES_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_UNSUITABLE_CANDIDATES_HH 1

#include <paludis/resolver/unsuitable_candidates-fwd.hh>
#include <paludis/resolver/constraint-fwd.hh>
#include <paludis/util/named_value.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/serialise-fwd.hh>

namespace paludis
{
    namespace n
    {
        typedef Name<struct name_package_id> package_id;
        typedef Name<struct name_unmet_constraints> unmet_constraints;
    }

    namespace resolver
    {
        struct UnsuitableCandidate
        {
            NamedValue<n::package_id, std::shared_ptr<const PackageID> > package_id;
            NamedValue<n::unmet_constraints, std::shared_ptr<const Constraints> > unmet_constraints;

            static UnsuitableCandidate deserialise(Deserialisation &) PALUDIS_ATTRIBUTE((warn_unused_result));
            void serialise(Serialiser &) const;
        };
    }
}

#endif
