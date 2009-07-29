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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_REASON_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_REASON_HH 1

#include <paludis/resolver/reason-fwd.hh>
#include <paludis/resolver/qpn_s-fwd.hh>
#include <paludis/resolver/sanitised_dependencies-fwd.hh>
#include <paludis/util/private_implementation_pattern.hh>

namespace paludis
{
    namespace resolver
    {
        class Reason
        {
            public:
                virtual ~Reason() = 0;
                virtual std::string as_string() const = 0;

                virtual DependencyReason * if_dependency_reason() = 0;
                virtual const DependencyReason * if_dependency_reason() const = 0;
        };

        class TargetReason :
            public Reason
        {
            public:
                virtual std::string as_string() const;
                virtual DependencyReason * if_dependency_reason();
                virtual const DependencyReason * if_dependency_reason() const;
        };

        class DependencyReason :
            private PrivateImplementationPattern<DependencyReason>,
            public Reason
        {
            public:
                DependencyReason(
                        const QPN_S & q,
                        const SanitisedDependency & s);

                ~DependencyReason();

                const QPN_S qpn_s() const;

                const SanitisedDependency & sanitised_dependency() const;

                virtual std::string as_string() const;

                virtual DependencyReason * if_dependency_reason();

                virtual const DependencyReason * if_dependency_reason() const;
        };
    }

#ifdef PALUDIS_HAVE_EXTERN_TEMPLATE
    extern template class PrivateImplementationPattern<resolver::DependencyReason>;
#endif

}

#endif
