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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_RESOLVER_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_RESOLVER_HH 1

#include <paludis/resolver/resolver-fwd.hh>
#include <paludis/resolver/reason-fwd.hh>
#include <paludis/resolver/resolver_functions-fwd.hh>
#include <paludis/resolver/decider-fwd.hh>
#include <paludis/resolver/orderer-fwd.hh>
#include <paludis/resolver/resolutions-fwd.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/named_value.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/name.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/repository-fwd.hh>
#include <paludis/spec_tree-fwd.hh>
#include <paludis/filtered_generator-fwd.hh>
#include <paludis/generator-fwd.hh>
#include <tr1/memory>

namespace paludis
{
    namespace n
    {
        struct all;
        struct errors;
        struct ordered;
        struct unordered;
        struct untaken;
    }

    namespace resolver
    {
        struct ResolverLists
        {
            NamedValue<n::all, std::tr1::shared_ptr<Resolutions> > all;
            NamedValue<n::errors, std::tr1::shared_ptr<Resolutions> > errors;
            NamedValue<n::ordered, std::tr1::shared_ptr<Resolutions> > ordered;
            NamedValue<n::unordered, std::tr1::shared_ptr<Resolutions> > unordered;
            NamedValue<n::untaken, std::tr1::shared_ptr<Resolutions> > untaken;

            static const ResolverLists deserialise(Deserialisation &) PALUDIS_ATTRIBUTE((warn_unused_result));
            void serialise(Serialiser &) const;
        };

        class PALUDIS_VISIBLE Resolver :
            private PrivateImplementationPattern<Resolver>
        {
            public:
                Resolver(
                        const Environment * const,
                        const ResolverFunctions &);
                ~Resolver();

                void add_target(const PackageDepSpec &);
                void add_target(const SetName &);

                void resolve();

                const std::tr1::shared_ptr<const ResolverLists> lists() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }
}

#endif
