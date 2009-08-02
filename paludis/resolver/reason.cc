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

std::string
PresetReason::as_string() const
{
    return "Preset()";
}

namespace paludis
{
    template <>
    struct Implementation<SetReason>
    {
        const SetName set_name;
        const std::tr1::shared_ptr<const Reason> reason_for_set;

        Implementation(const SetName & s, const std::tr1::shared_ptr<const Reason> & r) :
            set_name(s),
            reason_for_set(r)
        {
        }
    };
}

SetReason::SetReason(const SetName & s, const std::tr1::shared_ptr<const Reason> & r) :
    PrivateImplementationPattern<SetReason>(new Implementation<SetReason>(s, r))
{
}

SetReason::~SetReason()
{
}

const SetName
SetReason::set_name() const
{
    return _imp->set_name;
}

const std::tr1::shared_ptr<const Reason>
SetReason::reason_for_set() const
{
    return _imp->reason_for_set;
}

std::string
SetReason::as_string() const
{
    return "Set(set: " + stringify(_imp->set_name) + " because: " + stringify(*_imp->reason_for_set) + ")";
}

template class PrivateImplementationPattern<DependencyReason>;
template class PrivateImplementationPattern<SetReason>;

