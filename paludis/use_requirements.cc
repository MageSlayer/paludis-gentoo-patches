/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh
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

#include <paludis/use_requirements.hh>
#include <paludis/environment.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/tr1_memory.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/visitor-impl.hh>
#include <functional>
#include <list>

using namespace paludis;

template class ConstVisitor<UseRequirementVisitorTypes>;
template class ConstAcceptInterface<UseRequirementVisitorTypes>;

template class ConstAcceptInterfaceVisitsThis<UseRequirementVisitorTypes, EnabledUseRequirement>;
template class ConstAcceptInterfaceVisitsThis<UseRequirementVisitorTypes, DisabledUseRequirement>;
template class ConstAcceptInterfaceVisitsThis<UseRequirementVisitorTypes, EqualUseRequirement>;
template class ConstAcceptInterfaceVisitsThis<UseRequirementVisitorTypes, NotEqualUseRequirement>;

template class Visits<const EnabledUseRequirement>;
template class Visits<const DisabledUseRequirement>;
template class Visits<const EqualUseRequirement>;
template class Visits<const NotEqualUseRequirement>;

template class WrappedForwardIterator<UseRequirements::ConstIteratorTag, const tr1::shared_ptr<const UseRequirement> >;

namespace paludis
{
    template <>
    struct Implementation<UseRequirements>
    {
        std::list<tr1::shared_ptr<const UseRequirement> > reqs;
    };
}

UseRequirements::UseRequirements() :
    PrivateImplementationPattern<UseRequirements>(new Implementation<UseRequirements>)
{
}

UseRequirements::UseRequirements(const UseRequirements & other) :
    PrivateImplementationPattern<UseRequirements>(new Implementation<UseRequirements>(*other._imp.operator-> ()))
{
}

UseRequirements::~UseRequirements()
{
}

UseRequirements::ConstIterator
UseRequirements::begin() const
{
    return ConstIterator(_imp->reqs.begin());
}

UseRequirements::ConstIterator
UseRequirements::end() const
{
    return ConstIterator(_imp->reqs.end());
}

UseRequirements::ConstIterator
UseRequirements::find(const UseFlagName & u) const
{
    using namespace tr1::placeholders;
    return ConstIterator(std::find_if(_imp->reqs.begin(), _imp->reqs.end(),
                tr1::bind(std::equal_to<UseFlagName>(), u, tr1::bind(tr1::mem_fn(&UseRequirement::flag), _1))));
}

void
UseRequirements::insert(const tr1::shared_ptr<const UseRequirement> & req)
{
    _imp->reqs.push_back(req);
}

bool
UseRequirements::empty() const
{
    return _imp->reqs.empty();
}

UseRequirement::UseRequirement(const UseFlagName & n) :
    _name(n)
{
}

UseRequirement::~UseRequirement()
{
}

EnabledUseRequirement::EnabledUseRequirement(const UseFlagName & n) :
    UseRequirement(n)
{
}

EnabledUseRequirement::~EnabledUseRequirement()
{
}

bool
EnabledUseRequirement::satisfied_by(const Environment * const env, const PackageID & pkg) const
{
    return env->query_use(flag(), pkg);
}

DisabledUseRequirement::DisabledUseRequirement(const UseFlagName & n) :
    UseRequirement(n)
{
}

DisabledUseRequirement::~DisabledUseRequirement()
{
}

bool
DisabledUseRequirement::satisfied_by(const Environment * const env, const PackageID & pkg) const
{
    return ! env->query_use(flag(), pkg);
}

ConditionalUseRequirement::ConditionalUseRequirement(const UseFlagName & n, const tr1::shared_ptr<const PackageID> & i) :
    UseRequirement(n),
    _id(i)
{
}

ConditionalUseRequirement::~ConditionalUseRequirement()
{
}

IfMineThenUseRequirement::IfMineThenUseRequirement(const UseFlagName & n, const tr1::shared_ptr<const PackageID> & i) :
    ConditionalUseRequirement(n, i)
{
}

IfMineThenUseRequirement::~IfMineThenUseRequirement()
{
}

bool
IfMineThenUseRequirement::satisfied_by(const Environment * const env, const PackageID & pkg) const
{
    return ! env->query_use(flag(), *package_id()) || env->query_use(flag(), pkg);
}

IfNotMineThenUseRequirement::IfNotMineThenUseRequirement(const UseFlagName & n, const tr1::shared_ptr<const PackageID> & i) :
    ConditionalUseRequirement(n, i)
{
}

IfNotMineThenUseRequirement::~IfNotMineThenUseRequirement()
{
}

bool
IfNotMineThenUseRequirement::satisfied_by(const Environment * const env, const PackageID & pkg) const
{
    return env->query_use(flag(), *package_id()) || env->query_use(flag(), pkg);
}

IfMineThenNotUseRequirement::IfMineThenNotUseRequirement(const UseFlagName & n, const tr1::shared_ptr<const PackageID> & i) :
    ConditionalUseRequirement(n, i)
{
}

IfMineThenNotUseRequirement::~IfMineThenNotUseRequirement()
{
}

bool
IfMineThenNotUseRequirement::satisfied_by(const Environment * const env, const PackageID & pkg) const
{
    return ! env->query_use(flag(), *package_id()) || ! env->query_use(flag(), pkg);
}

IfNotMineThenNotUseRequirement::IfNotMineThenNotUseRequirement(const UseFlagName & n, const tr1::shared_ptr<const PackageID> & i) :
    ConditionalUseRequirement(n, i)
{
}

IfNotMineThenNotUseRequirement::~IfNotMineThenNotUseRequirement()
{
}

bool
IfNotMineThenNotUseRequirement::satisfied_by(const Environment * const env, const PackageID & pkg) const
{
    return env->query_use(flag(), *package_id()) || ! env->query_use(flag(), pkg);
}

EqualUseRequirement::EqualUseRequirement(const UseFlagName & n, const tr1::shared_ptr<const PackageID> & i) :
    ConditionalUseRequirement(n, i)
{
}

EqualUseRequirement::~EqualUseRequirement()
{
}

bool
EqualUseRequirement::satisfied_by(const Environment * const env, const PackageID & pkg) const
{
    return env->query_use(flag(), pkg) == env->query_use(flag(), *package_id());
}

NotEqualUseRequirement::NotEqualUseRequirement(const UseFlagName & n, const tr1::shared_ptr<const PackageID> & i) :
    ConditionalUseRequirement(n, i)
{
}

NotEqualUseRequirement::~NotEqualUseRequirement()
{
}

bool
NotEqualUseRequirement::satisfied_by(const Environment * const env, const PackageID & pkg) const
{
    return env->query_use(flag(), pkg) != env->query_use(flag(), *package_id());
}

