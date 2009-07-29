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

#include <paludis/resolver/reason.hh>
#include <paludis/resolver/qpn_s.hh>
#include <paludis/resolver/sanitised_dependencies.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>

using namespace paludis;
using namespace paludis::resolver;

std::ostream &
paludis::resolver::operator<< (std::ostream & s, const Reason & r)
{
    s << r.as_string();
    return s;
}

Reason::~Reason()
{
}

std::string
TargetReason::as_string() const
{
    return "Target()";
}

DependencyReason *
TargetReason::if_dependency_reason()
{
    return 0;
}

const DependencyReason *
TargetReason::if_dependency_reason() const
{
    return 0;
}

namespace paludis
{
    template <>
    struct Implementation<DependencyReason>
    {
        const QPN_S qpn_s;
        const SanitisedDependency dep;

        Implementation(const QPN_S & q, const SanitisedDependency & d) :
            qpn_s(q),
            dep(d)
        {
        }
    };
}

DependencyReason::DependencyReason(const QPN_S & q, const SanitisedDependency & d) :
    PrivateImplementationPattern<DependencyReason>(new Implementation<DependencyReason>(q, d))
{
}

DependencyReason::~DependencyReason()
{
}

const QPN_S
DependencyReason::qpn_s() const
{
    return _imp->qpn_s;
}

const SanitisedDependency &
DependencyReason::sanitised_dependency() const
{
    return _imp->dep;
}

std::string
DependencyReason::as_string() const
{
    return "Dependency(package: " + stringify(_imp->qpn_s) + " dep: " + stringify(_imp->dep) + ")";
}

DependencyReason *
DependencyReason::if_dependency_reason()
{
    return this;
}

const DependencyReason *
DependencyReason::if_dependency_reason() const
{
    return this;
}

template class PrivateImplementationPattern<DependencyReason>;

