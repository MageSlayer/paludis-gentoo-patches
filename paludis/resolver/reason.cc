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

#include <paludis/resolver/reason.hh>
#include <paludis/resolver/resolvent.hh>
#include <paludis/resolver/sanitised_dependencies.hh>
#include <paludis/resolver/change_by_resolvent.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/serialise-impl.hh>

using namespace paludis;
using namespace paludis::resolver;

Reason::~Reason()
{
}

namespace paludis
{
    template <>
    struct Implementation<TargetReason>
    {
        const std::string extra_information;

        Implementation(const std::string & x) :
            extra_information(x)
        {
        }
    };
}

TargetReason::TargetReason(const std::string & x) :
    PrivateImplementationPattern<TargetReason>(new Implementation<TargetReason>(x))
{
}

TargetReason::~TargetReason()
{
}

const std::string
TargetReason::extra_information() const
{
    return _imp->extra_information;
}

void
TargetReason::serialise(Serialiser & s) const
{
    s.object("TargetReason")
        .member(SerialiserFlags<>(), "extra_information", extra_information())
        ;
}

namespace paludis
{
    template <>
    struct Implementation<DependencyReason>
    {
        const std::shared_ptr<const PackageID> from_id;
        const Resolvent from_resolvent;
        const SanitisedDependency dep;
        const bool already_met;

        Implementation(const std::shared_ptr<const PackageID> & i,
                const Resolvent & r, const SanitisedDependency & d, const bool a) :
            from_id(i),
            from_resolvent(r),
            dep(d),
            already_met(a)
        {
        }
    };
}

DependencyReason::DependencyReason(const std::shared_ptr<const PackageID> & i,
        const Resolvent & r,
        const SanitisedDependency & d,
        const bool a) :
    PrivateImplementationPattern<DependencyReason>(new Implementation<DependencyReason>(i, r, d, a))
{
}

DependencyReason::~DependencyReason()
{
}

const std::shared_ptr<const PackageID>
DependencyReason::from_id() const
{
    return _imp->from_id;
}

const Resolvent
DependencyReason::from_resolvent() const
{
    return _imp->from_resolvent;
}

const SanitisedDependency &
DependencyReason::sanitised_dependency() const
{
    return _imp->dep;
}

bool
DependencyReason::already_met() const
{
    return _imp->already_met;
}

void
DependencyReason::serialise(Serialiser & s) const
{
    s.object("DependencyReason")
        .member(SerialiserFlags<>(), "already_met", already_met())
        .member(SerialiserFlags<serialise::might_be_null>(), "from_id", from_id())
        .member(SerialiserFlags<>(), "from_resolvent", from_resolvent())
        .member(SerialiserFlags<>(), "sanitised_dependency", sanitised_dependency())
        ;
}

namespace paludis
{
    template <>
    struct Implementation<DependentReason>
    {
        const ChangeByResolvent id_being_removed;

        Implementation(const ChangeByResolvent & i) :
            id_being_removed(i)
        {
        }
    };
}

DependentReason::DependentReason(const ChangeByResolvent & i) :
    PrivateImplementationPattern<DependentReason>(new Implementation<DependentReason>(i))
{
}

DependentReason::~DependentReason()
{
}

const ChangeByResolvent
DependentReason::id_and_resolvent_being_removed() const
{
    return _imp->id_being_removed;
}

void
DependentReason::serialise(Serialiser & s) const
{
    s.object("DependentReason")
        .member(SerialiserFlags<>(), "id_and_resolvent_being_removed", id_and_resolvent_being_removed())
        ;
}

namespace paludis
{
    template <>
    struct Implementation<WasUsedByReason>
    {
        const std::shared_ptr<const ChangeByResolventSequence> ids_being_removed;

        Implementation(const std::shared_ptr<const ChangeByResolventSequence> & i) :
            ids_being_removed(i)
        {
        }
    };
}

WasUsedByReason::WasUsedByReason(const std::shared_ptr<const ChangeByResolventSequence> & i) :
    PrivateImplementationPattern<WasUsedByReason>(new Implementation<WasUsedByReason>(i))
{
}

WasUsedByReason::~WasUsedByReason()
{
}

const std::shared_ptr<const ChangeByResolventSequence>
WasUsedByReason::ids_and_resolvents_being_removed() const
{
    return _imp->ids_being_removed;
}

void
WasUsedByReason::serialise(Serialiser & s) const
{
    s.object("WasUsedByReason")
        .member(SerialiserFlags<serialise::container, serialise::might_be_null>(), "ids_and_resolvents_being_removed", ids_and_resolvents_being_removed())
        ;
}

namespace paludis
{
    template <>
    struct Implementation<PresetReason>
    {
        const std::string explanation;
        const std::shared_ptr<const Reason> reason_for_preset;

        Implementation(const std::string & m, const std::shared_ptr<const Reason> & r) :
            explanation(m),
            reason_for_preset(r)
        {
        }
    };
}

PresetReason::PresetReason(const std::string & m, const std::shared_ptr<const Reason> & r) :
    PrivateImplementationPattern<PresetReason>(new Implementation<PresetReason>(m, r))
{
}

PresetReason::~PresetReason()
{
}

const std::shared_ptr<const Reason>
PresetReason::maybe_reason_for_preset() const
{
    return _imp->reason_for_preset;
}

const std::string
PresetReason::maybe_explanation() const
{
    return _imp->explanation;
}

void
PresetReason::serialise(Serialiser & s) const
{
    s.object("PresetReason")
        .member(SerialiserFlags<>(), "maybe_explanation", maybe_explanation())
        .member(SerialiserFlags<serialise::might_be_null>(), "maybe_reason_for_preset", maybe_reason_for_preset())
        ;
}

namespace paludis
{
    template <>
    struct Implementation<SetReason>
    {
        const SetName set_name;
        const std::shared_ptr<const Reason> reason_for_set;

        Implementation(const SetName & s, const std::shared_ptr<const Reason> & r) :
            set_name(s),
            reason_for_set(r)
        {
        }
    };
}

SetReason::SetReason(const SetName & s, const std::shared_ptr<const Reason> & r) :
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

const std::shared_ptr<const Reason>
SetReason::reason_for_set() const
{
    return _imp->reason_for_set;
}

void
SetReason::serialise(Serialiser & s) const
{
    s.object("SetReason")
        .member(SerialiserFlags<serialise::might_be_null>(), "reason_for_set", reason_for_set())
        .member(SerialiserFlags<>(), "set_name", stringify(set_name()))
        ;
}

namespace paludis
{
    template <>
    struct Implementation<LikeOtherDestinationTypeReason>
    {
        const Resolvent other_resolvent;
        const std::shared_ptr<const Reason> reason_for_other;

        Implementation(const Resolvent & s, const std::shared_ptr<const Reason> & r) :
            other_resolvent(s),
            reason_for_other(r)
        {
        }
    };
}

LikeOtherDestinationTypeReason::LikeOtherDestinationTypeReason(const Resolvent & s, const std::shared_ptr<const Reason> & r) :
    PrivateImplementationPattern<LikeOtherDestinationTypeReason>(new Implementation<LikeOtherDestinationTypeReason>(s, r))
{
}

LikeOtherDestinationTypeReason::~LikeOtherDestinationTypeReason()
{
}

const Resolvent
LikeOtherDestinationTypeReason::other_resolvent() const
{
    return _imp->other_resolvent;
}

const std::shared_ptr<const Reason>
LikeOtherDestinationTypeReason::reason_for_other() const
{
    return _imp->reason_for_other;
}

void
LikeOtherDestinationTypeReason::serialise(Serialiser & s) const
{
    s.object("LikeOtherDestinationTypeReason")
        .member(SerialiserFlags<serialise::might_be_null>(), "reason_for_other", reason_for_other())
        .member(SerialiserFlags<>(), "other_resolvent", other_resolvent())
        ;
}

namespace paludis
{
    template <>
    struct Implementation<ViaBinaryReason>
    {
        const Resolvent other_resolvent;

        Implementation(const Resolvent & s) :
            other_resolvent(s)
        {
        }
    };
}

ViaBinaryReason::ViaBinaryReason(const Resolvent & r) :
    PrivateImplementationPattern<ViaBinaryReason>(new Implementation<ViaBinaryReason>(r))
{
}

ViaBinaryReason::~ViaBinaryReason()
{
}

const Resolvent
ViaBinaryReason::other_resolvent() const
{
    return _imp->other_resolvent;
}

void
ViaBinaryReason::serialise(Serialiser & s) const
{
    s.object("ViaBinaryReason")
        .member(SerialiserFlags<>(), "other_resolvent", other_resolvent())
        ;
}

const std::shared_ptr<Reason>
Reason::deserialise(Deserialisation & d)
{
    if (d.class_name() == "TargetReason")
    {
        Deserialisator v(d, "TargetReason");
        return make_shared_ptr(new TargetReason(v.member<std::string>("extra_information")));
    }
    else if (d.class_name() == "PresetReason")
    {
        Deserialisator v(d, "PresetReason");
        return make_shared_ptr(new PresetReason(
                    v.member<std::string>("maybe_explanation"),
                    v.member<std::shared_ptr<Reason> >("maybe_reason_for_preset")
                    ));
    }
    else if (d.class_name() == "SetReason")
    {
        Deserialisator v(d, "SetReason");
        return make_shared_ptr(new SetReason(
                    SetName(v.member<std::string>("set_name")),
                    v.member<std::shared_ptr<Reason> >("reason_for_set")
                    ));
    }
    else if (d.class_name() == "DependencyReason")
    {
        Deserialisator v(d, "DependencyReason");
        const std::shared_ptr<const PackageID> from_id(v.member<std::shared_ptr<const PackageID> >("from_id"));
        return make_shared_ptr(new DependencyReason(
                    from_id,
                    v.member<Resolvent>("from_resolvent"),
                    SanitisedDependency::deserialise(*v.find_remove_member("sanitised_dependency"), from_id),
                    v.member<bool>("already_met"))
                );
    }
    else if (d.class_name() == "DependentReason")
    {
        Deserialisator v(d, "DependentReason");
        return make_shared_ptr(new DependentReason(
                    v.member<ChangeByResolvent>("id_and_resolvent_being_removed"))
                );
    }
    else if (d.class_name() == "WasUsedByReason")
    {
        Deserialisator v(d, "WasUsedByReason");
        Deserialisator vv(*v.find_remove_member("ids_and_resolvents_being_removed"), "c");
        std::shared_ptr<ChangeByResolventSequence> ids_and_resolvents_being_removed(new ChangeByResolventSequence);
        for (int n(1), n_end(vv.member<int>("count") + 1) ; n != n_end ; ++n)
            ids_and_resolvents_being_removed->push_back(vv.member<ChangeByResolvent>(stringify(n)));
        return make_shared_ptr(new WasUsedByReason(ids_and_resolvents_being_removed));
    }
    else if (d.class_name() == "LikeOtherDestinationTypeReason")
    {
        Deserialisator v(d, "LikeOtherDestinationTypeReason");
        return make_shared_ptr(new LikeOtherDestinationTypeReason(
                    v.member<Resolvent>("other_resolvent"),
                    v.member<std::shared_ptr<Reason> >("reason_for_other")
                    ));
    }
    else if (d.class_name() == "ViaBinaryReason")
    {
        Deserialisator v(d, "ViaBinaryReason");
        return make_shared_ptr(new ViaBinaryReason(
                    v.member<Resolvent>("other_resolvent")
                    ));
    }
    else
        throw InternalError(PALUDIS_HERE, "unknown class '" + stringify(d.class_name()) + "'");
}

template class PrivateImplementationPattern<TargetReason>;
template class PrivateImplementationPattern<DependencyReason>;
template class PrivateImplementationPattern<DependentReason>;
template class PrivateImplementationPattern<SetReason>;
template class PrivateImplementationPattern<PresetReason>;
template class PrivateImplementationPattern<WasUsedByReason>;
template class PrivateImplementationPattern<LikeOtherDestinationTypeReason>;
template class PrivateImplementationPattern<ViaBinaryReason>;

