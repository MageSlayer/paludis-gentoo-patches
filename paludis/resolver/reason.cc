/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010, 2011 Ciaran McCreesh
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
#include <paludis/resolver/collect_depped_upon.hh>

#include <paludis/util/stringify.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>

#include <paludis/serialise-impl.hh>
#include <paludis/changed_choices.hh>

using namespace paludis;
using namespace paludis::resolver;

Reason::~Reason()
{
}

namespace paludis
{
    template <>
    struct Imp<TargetReason>
    {
        const std::string extra_information;

        Imp(const std::string & x) :
            extra_information(x)
        {
        }
    };
}

TargetReason::TargetReason(const std::string & x) :
    _imp(x)
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
    struct Imp<DependencyReason>
    {
        const std::shared_ptr<const PackageID> from_id;
        const std::shared_ptr<const ChangedChoices> changed_choices;
        const Resolvent from_resolvent;
        const SanitisedDependency dep;
        const Tribool already_met;

        Imp(const std::shared_ptr<const PackageID> & i,
                const std::shared_ptr<const ChangedChoices> & c,
                const Resolvent & r, const SanitisedDependency & d, const Tribool a) :
            from_id(i),
            changed_choices(c),
            from_resolvent(r),
            dep(d),
            already_met(a)
        {
        }
    };
}

DependencyReason::DependencyReason(const std::shared_ptr<const PackageID> & i,
        const std::shared_ptr<const ChangedChoices> & c,
        const Resolvent & r,
        const SanitisedDependency & d,
        const Tribool a) :
    _imp(i, c, r, d, a)
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

const std::shared_ptr<const ChangedChoices>
DependencyReason::from_id_changed_choices() const
{
    return _imp->changed_choices;
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

Tribool
DependencyReason::already_met() const
{
    return _imp->already_met;
}

void
DependencyReason::serialise(Serialiser & s) const
{
    s.object("DependencyReason")
        .member(SerialiserFlags<>(), "already_met", std::string(already_met().is_true() ? "true" : already_met().is_false() ? "false" : "indeterminate"))
        .member(SerialiserFlags<serialise::might_be_null>(), "from_id", from_id())
        .member(SerialiserFlags<serialise::might_be_null>(), "from_id_changed_choices", from_id_changed_choices())
        .member(SerialiserFlags<>(), "from_resolvent", from_resolvent())
        .member(SerialiserFlags<>(), "sanitised_dependency", sanitised_dependency())
        ;
}

namespace paludis
{
    template <>
    struct Imp<DependentReason>
    {
        const DependentPackageID dependent_upon;

        Imp(const DependentPackageID & i) :
            dependent_upon(i)
        {
        }
    };
}

DependentReason::DependentReason(const DependentPackageID & i) :
    _imp(i)
{
}

DependentReason::~DependentReason()
{
}

const DependentPackageID
DependentReason::dependent_upon() const
{
    return _imp->dependent_upon;
}

void
DependentReason::serialise(Serialiser & s) const
{
    s.object("DependentReason")
        .member(SerialiserFlags<>(), "dependent_upon", dependent_upon())
        ;
}

namespace paludis
{
    template <>
    struct Imp<WasUsedByReason>
    {
        const std::shared_ptr<const ChangeByResolventSequence> ids_being_removed;

        Imp(const std::shared_ptr<const ChangeByResolventSequence> & i) :
            ids_being_removed(i)
        {
        }
    };
}

WasUsedByReason::WasUsedByReason(const std::shared_ptr<const ChangeByResolventSequence> & i) :
    _imp(i)
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
    struct Imp<PresetReason>
    {
        const std::string explanation;
        const std::shared_ptr<const Reason> reason_for_preset;

        Imp(const std::string & m, const std::shared_ptr<const Reason> & r) :
            explanation(m),
            reason_for_preset(r)
        {
        }
    };
}

PresetReason::PresetReason(const std::string & m, const std::shared_ptr<const Reason> & r) :
    _imp(m, r)
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
    struct Imp<SetReason>
    {
        const SetName set_name;
        const std::shared_ptr<const Reason> reason_for_set;

        Imp(const SetName & s, const std::shared_ptr<const Reason> & r) :
            set_name(s),
            reason_for_set(r)
        {
        }
    };
}

SetReason::SetReason(const SetName & s, const std::shared_ptr<const Reason> & r) :
    _imp(s, r)
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
    struct Imp<LikeOtherDestinationTypeReason>
    {
        const Resolvent other_resolvent;
        const std::shared_ptr<const Reason> reason_for_other;

        Imp(const Resolvent & s, const std::shared_ptr<const Reason> & r) :
            other_resolvent(s),
            reason_for_other(r)
        {
        }
    };
}

LikeOtherDestinationTypeReason::LikeOtherDestinationTypeReason(const Resolvent & s, const std::shared_ptr<const Reason> & r) :
    _imp(s, r)
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
    struct Imp<ViaBinaryReason>
    {
        const Resolvent other_resolvent;

        Imp(const Resolvent & s) :
            other_resolvent(s)
        {
        }
    };
}

ViaBinaryReason::ViaBinaryReason(const Resolvent & r) :
    _imp(r)
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

namespace
{
    Tribool destringify_tribool(const std::string & s)
    {
        if (s == "true")
            return true;
        if (s == "false")
            return false;
        if (s == "indeterminate")
            return indeterminate;
        throw InternalError(PALUDIS_HERE, "bad tribool " + s);
    }
}

const std::shared_ptr<Reason>
Reason::deserialise(Deserialisation & d)
{
    if (d.class_name() == "TargetReason")
    {
        Deserialisator v(d, "TargetReason");
        return std::make_shared<TargetReason>(v.member<std::string>("extra_information"));
    }
    else if (d.class_name() == "PresetReason")
    {
        Deserialisator v(d, "PresetReason");
        return std::make_shared<PresetReason>(
                    v.member<std::string>("maybe_explanation"),
                    v.member<std::shared_ptr<Reason> >("maybe_reason_for_preset")
                    );
    }
    else if (d.class_name() == "SetReason")
    {
        Deserialisator v(d, "SetReason");
        return std::make_shared<SetReason>(
                    SetName(v.member<std::string>("set_name")),
                    v.member<std::shared_ptr<Reason> >("reason_for_set")
                    );
    }
    else if (d.class_name() == "DependencyReason")
    {
        Deserialisator v(d, "DependencyReason");
        const std::shared_ptr<const PackageID> from_id(v.member<std::shared_ptr<const PackageID> >("from_id"));
        return std::make_shared<DependencyReason>(
                    from_id,
                    v.member<std::shared_ptr<const ChangedChoices> >("from_id_changed_choices"),
                    v.member<Resolvent>("from_resolvent"),
                    SanitisedDependency::deserialise(*v.find_remove_member("sanitised_dependency"), from_id),
                    destringify_tribool(v.member<std::string>("already_met"))
                );
    }
    else if (d.class_name() == "DependentReason")
    {
        Deserialisator v(d, "DependentReason");
        return std::make_shared<DependentReason>(
                    v.member<DependentPackageID>("dependent_upon")
                );
    }
    else if (d.class_name() == "WasUsedByReason")
    {
        Deserialisator v(d, "WasUsedByReason");
        Deserialisator vv(*v.find_remove_member("ids_and_resolvents_being_removed"), "c");
        std::shared_ptr<ChangeByResolventSequence> ids_and_resolvents_being_removed(std::make_shared<ChangeByResolventSequence>());
        for (int n(1), n_end(vv.member<int>("count") + 1) ; n != n_end ; ++n)
            ids_and_resolvents_being_removed->push_back(vv.member<ChangeByResolvent>(stringify(n)));
        return std::make_shared<WasUsedByReason>(ids_and_resolvents_being_removed);
    }
    else if (d.class_name() == "LikeOtherDestinationTypeReason")
    {
        Deserialisator v(d, "LikeOtherDestinationTypeReason");
        return std::make_shared<LikeOtherDestinationTypeReason>(
                    v.member<Resolvent>("other_resolvent"),
                    v.member<std::shared_ptr<Reason> >("reason_for_other")
                    );
    }
    else if (d.class_name() == "ViaBinaryReason")
    {
        Deserialisator v(d, "ViaBinaryReason");
        return std::make_shared<ViaBinaryReason>(
                    v.member<Resolvent>("other_resolvent")
                    );
    }
    else
        throw InternalError(PALUDIS_HERE, "unknown class '" + stringify(d.class_name()) + "'");
}

namespace paludis
{
    template class Pimp<TargetReason>;
    template class Pimp<DependencyReason>;
    template class Pimp<DependentReason>;
    template class Pimp<SetReason>;
    template class Pimp<PresetReason>;
    template class Pimp<WasUsedByReason>;
    template class Pimp<LikeOtherDestinationTypeReason>;
    template class Pimp<ViaBinaryReason>;

    template class Sequence<std::shared_ptr<const Reason> >;
    template class WrappedForwardIterator<Reasons::ConstIteratorTag, const std::shared_ptr<const Reason> >;
}
