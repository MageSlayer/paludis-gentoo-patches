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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_CONSTRAINT_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_CONSTRAINT_HH 1

#include <paludis/resolver/constraint-fwd.hh>
#include <paludis/resolver/reason-fwd.hh>
#include <paludis/resolver/use_installed-fwd.hh>
#include <paludis/resolver/sanitised_dependencies.hh>
#include <paludis/util/named_value.hh>
#include <paludis/dep_spec.hh>
#include <tr1/memory>

namespace paludis
{
    namespace n
    {
        struct nothing_is_fine_too;
        struct reason;
        struct spec;
        struct to_destination_slash;
        struct use_installed;
    }

    namespace resolver
    {
        struct Constraint
        {
            NamedValue<n::nothing_is_fine_too, bool> nothing_is_fine_too;
            NamedValue<n::reason, std::tr1::shared_ptr<const Reason> > reason;
            NamedValue<n::spec, PackageOrBlockDepSpec> spec;
            NamedValue<n::to_destination_slash, bool> to_destination_slash;
            NamedValue<n::use_installed, UseInstalled> use_installed;
        };

        class PALUDIS_VISIBLE Constraints :
            private PrivateImplementationPattern<Constraints>
        {
            public:
                Constraints();
                ~Constraints();

                UseInstalled strictest_use_installed() const PALUDIS_ATTRIBUTE((warn_unused_result));
                bool nothing_is_fine_too() const PALUDIS_ATTRIBUTE((warn_unused_result));
                bool to_destination_slash() const PALUDIS_ATTRIBUTE((warn_unused_result));

                void add(const std::tr1::shared_ptr<const Constraint> &);

                struct ConstIteratorTag;
                typedef WrappedForwardIterator<ConstIteratorTag, const std::tr1::shared_ptr<const Constraint> > ConstIterator;
                ConstIterator begin() const PALUDIS_ATTRIBUTE((warn_unused_result));
                ConstIterator end() const PALUDIS_ATTRIBUTE((warn_unused_result));

                bool empty() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }

#ifdef PALUDIS_HAVE_EXTERN_TEMPLATE
    extern template class PrivateImplementationPattern<resolver::Constraints>;
#endif
}

#endif
