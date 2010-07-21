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

#include <paludis/resolver/decision.hh>
#include <paludis/resolver/destination.hh>
#include <paludis/resolver/unsuitable_candidates.hh>
#include <paludis/resolver/resolvent.hh>
#include <paludis/resolver/required_confirmations.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/make_shared_copy.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/serialise-impl.hh>
#include <sstream>

using namespace paludis;
using namespace paludis::resolver;

Decision::~Decision() = default;

const std::tr1::shared_ptr<Decision>
Decision::deserialise(Deserialisation & d)
{
    if (d.class_name() == "NothingNoChangeDecision")
    {
        Deserialisator v(d, "NothingNoChangeDecision");
        return make_shared_ptr(new NothingNoChangeDecision(
                    v.member<Resolvent>("resolvent"),
                    v.member<bool>("taken")
                    ));
    }
    else if (d.class_name() == "ExistingNoChangeDecision")
    {
        Deserialisator v(d, "ExistingNoChangeDecision");
        return make_shared_ptr(new ExistingNoChangeDecision(
                    v.member<Resolvent>("resolvent"),
                    v.member<std::tr1::shared_ptr<const PackageID> >("existing_id"),
                    v.member<bool>("is_same"),
                    v.member<bool>("is_same_version"),
                    v.member<bool>("is_transient"),
                    v.member<bool>("taken")
                    ));
    }
    else if (d.class_name() == "ChangesToMakeDecision")
    {
        return ChangesToMakeDecision::deserialise(d);
    }
    else if (d.class_name() == "UnableToMakeDecision")
    {
        return UnableToMakeDecision::deserialise(d);
    }
    else if (d.class_name() == "RemoveDecision")
    {
        return RemoveDecision::deserialise(d);
    }
    else if (d.class_name() == "BreakDecision")
    {
        return BreakDecision::deserialise(d);
    }
    else
        throw InternalError(PALUDIS_HERE, "unknown class '" + stringify(d.class_name()) + "'");
}

const std::tr1::shared_ptr<ConfirmableDecision>
ConfirmableDecision::deserialise(Deserialisation & d)
{
    if (d.class_name() == "ChangesToMakeDecision")
    {
        return ChangesToMakeDecision::deserialise(d);
    }
    else if (d.class_name() == "RemoveDecision")
    {
        return RemoveDecision::deserialise(d);
    }
    else if (d.class_name() == "BreakDecision")
    {
        return BreakDecision::deserialise(d);
    }
    else
        throw InternalError(PALUDIS_HERE, "unknown class '" + stringify(d.class_name()) + "'");
}

const std::tr1::shared_ptr<ChangeOrRemoveDecision>
ChangeOrRemoveDecision::deserialise(Deserialisation & d)
{
    if (d.class_name() == "ChangesToMakeDecision")
    {
        return ChangesToMakeDecision::deserialise(d);
    }
    else if (d.class_name() == "RemoveDecision")
    {
        return RemoveDecision::deserialise(d);
    }
    else
        throw InternalError(PALUDIS_HERE, "unknown class '" + stringify(d.class_name()) + "'");
}

const std::tr1::shared_ptr<ChangesToMakeDecision>
ChangesToMakeDecision::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "ChangesToMakeDecision");
    std::tr1::shared_ptr<ChangesToMakeDecision> result(new ChangesToMakeDecision(
                v.member<Resolvent>("resolvent"),
                v.member<std::tr1::shared_ptr<const PackageID> >("origin_id"),
                v.member<bool>("best"),
                destringify<ChangeType>(v.member<std::string>("change_type")),
                v.member<bool>("taken"),
                v.member<std::tr1::shared_ptr<const Destination> >("destination"),
                std::tr1::function<void (const ChangesToMakeDecision &)>()
                ));

    {
        const std::tr1::shared_ptr<Deserialisation> dn(v.find_remove_member("required_confirmations_if_any"));
        if (! dn->null())
        {
            Deserialisator vv(*dn, "c");
            for (int n(1), n_end(vv.member<int>("count") + 1) ; n != n_end ; ++n)
                result->add_required_confirmation(vv.member<std::tr1::shared_ptr<RequiredConfirmation> >(stringify(n)));
        }
    }

    {
        const std::tr1::shared_ptr<Deserialisation> dn(v.find_remove_member("if_via_new_binary_in"));
        if (! dn->null())
            result->set_via_new_binary_in(RepositoryName(dn->string_value()));
    }

    return result;
}

const std::tr1::shared_ptr<UnableToMakeDecision>
UnableToMakeDecision::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "UnableToMakeDecision");

    std::tr1::shared_ptr<UnsuitableCandidates> unsuitable_candidates(new UnsuitableCandidates);
    Deserialisator vv(*v.find_remove_member("unsuitable_candidates"), "c");
    for (int n(1), n_end(vv.member<int>("count") + 1) ; n != n_end ; ++n)
        unsuitable_candidates->push_back(vv.member<UnsuitableCandidate>(stringify(n)));

    return make_shared_ptr(new UnableToMakeDecision(
                v.member<Resolvent>("resolvent"),
                unsuitable_candidates,
                v.member<bool>("taken")
                ));
}

const std::tr1::shared_ptr<RemoveDecision>
RemoveDecision::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "RemoveDecision");

    std::tr1::shared_ptr<PackageIDSequence> ids(new PackageIDSequence);
    {
        Deserialisator vv(*v.find_remove_member("ids"), "c");
        for (int n(1), n_end(vv.member<int>("count") + 1) ; n != n_end ; ++n)
            ids->push_back(vv.member<std::tr1::shared_ptr<const PackageID> >(stringify(n)));
    }

    const std::tr1::shared_ptr<RemoveDecision> result(new RemoveDecision(
                v.member<Resolvent>("resolvent"),
                ids,
                v.member<bool>("taken")
                ));

    {
        const std::tr1::shared_ptr<Deserialisation> dn(v.find_remove_member("required_confirmations_if_any"));
        if (! dn->null())
        {
            Deserialisator vv(*dn, "c");
            for (int n(1), n_end(vv.member<int>("count") + 1) ; n != n_end ; ++n)
                result->add_required_confirmation(vv.member<std::tr1::shared_ptr<RequiredConfirmation> >(stringify(n)));
        }
    }

    return result;
}

namespace paludis
{
    template <>
    struct Implementation<NothingNoChangeDecision>
    {
        const Resolvent resolvent;
        const bool taken;

        Implementation(const Resolvent & r, const bool t) :
            resolvent(r),
            taken(t)
        {
        }
    };
}

NothingNoChangeDecision::NothingNoChangeDecision(const Resolvent & r, const bool t) :
    PrivateImplementationPattern<NothingNoChangeDecision>(new Implementation<NothingNoChangeDecision>(r, t))
{
}

NothingNoChangeDecision::~NothingNoChangeDecision() = default;

const Resolvent
NothingNoChangeDecision::resolvent() const
{
    return _imp->resolvent;
}

bool
NothingNoChangeDecision::taken() const
{
    return _imp->taken;
}

void
NothingNoChangeDecision::serialise(Serialiser & s) const
{
    s.object("NothingNoChangeDecision")
        .member(SerialiserFlags<>(), "resolvent", resolvent())
        .member(SerialiserFlags<>(), "taken", taken())
        ;
}

namespace paludis
{
    template <>
    struct Implementation<ExistingNoChangeDecision>
    {
        const Resolvent resolvent;
        const std::tr1::shared_ptr<const PackageID> existing_id;
        const bool is_same;
        const bool is_same_version;
        const bool is_transient;
        const bool taken;

        Implementation(const Resolvent & l,
                const std::tr1::shared_ptr<const PackageID> & e,
                const bool s, const bool v, const bool r, const bool t) :
            resolvent(l),
            existing_id(e),
            is_same(s),
            is_same_version(v),
            is_transient(r),
            taken(t)
        {
        }
    };
}

ExistingNoChangeDecision::ExistingNoChangeDecision(const Resolvent & l, const std::tr1::shared_ptr<const PackageID> & e,
        const bool s, const bool v, const bool r, const bool t) :
    PrivateImplementationPattern<ExistingNoChangeDecision>(new Implementation<ExistingNoChangeDecision>(
                l, e, s, v, r, t))
{
}

ExistingNoChangeDecision::~ExistingNoChangeDecision() = default;

const std::tr1::shared_ptr<const PackageID>
ExistingNoChangeDecision::existing_id() const
{
    return _imp->existing_id;
}

bool
ExistingNoChangeDecision::is_same() const
{
    return _imp->is_same;
}

bool
ExistingNoChangeDecision::is_same_version() const
{
    return _imp->is_same_version;
}

bool
ExistingNoChangeDecision::is_transient() const
{
    return _imp->is_transient;
}

const Resolvent
ExistingNoChangeDecision::resolvent() const
{
    return _imp->resolvent;
}

bool
ExistingNoChangeDecision::taken() const
{
    return _imp->taken;
}

void
ExistingNoChangeDecision::serialise(Serialiser & s) const
{
    s.object("ExistingNoChangeDecision")
        .member(SerialiserFlags<>(), "resolvent", resolvent())
        .member(SerialiserFlags<serialise::might_be_null>(), "existing_id", existing_id())
        .member(SerialiserFlags<>(), "is_same", is_same())
        .member(SerialiserFlags<>(), "is_same_version", is_same_version())
        .member(SerialiserFlags<>(), "is_transient", is_transient())
        .member(SerialiserFlags<>(), "taken", taken())
        ;
}

namespace paludis
{
    template <>
    struct Implementation<ChangesToMakeDecision>
    {
        const Resolvent resolvent;
        const std::tr1::shared_ptr<const PackageID> origin_id;
        const bool best;
        ChangeType change_type;
        const bool taken;
        std::tr1::shared_ptr<const Destination> destination;
        std::tr1::shared_ptr<RequiredConfirmations> required_confirmations;
        std::tr1::shared_ptr<RepositoryName> if_via_new_binary_in;

        Implementation(
                const Resolvent & l,
                const std::tr1::shared_ptr<const PackageID> & o,
                const bool b,
                const ChangeType c,
                const bool t,
                const std::tr1::shared_ptr<const Destination> & d) :
            resolvent(l),
            origin_id(o),
            best(b),
            change_type(c),
            taken(t),
            destination(d)
        {
        }
    };
}

ChangesToMakeDecision::ChangesToMakeDecision(
        const Resolvent & r,
        const std::tr1::shared_ptr<const PackageID> & o,
        const bool b,
        const ChangeType c,
        const bool t,
        const std::tr1::shared_ptr<const Destination> & d,
        const std::tr1::function<void (ChangesToMakeDecision &)> & f) :
    PrivateImplementationPattern<ChangesToMakeDecision>(new Implementation<ChangesToMakeDecision>(r, o, b, c, t, d))
{
    if (f)
        f(*this);
}

ChangesToMakeDecision::~ChangesToMakeDecision() = default;

const std::tr1::shared_ptr<const Destination>
ChangesToMakeDecision::destination() const
{
    return _imp->destination;
}

void
ChangesToMakeDecision::set_destination(const std::tr1::shared_ptr<const Destination> & d)
{
    _imp->destination = d;
}

const std::tr1::shared_ptr<const RequiredConfirmations>
ChangesToMakeDecision::required_confirmations_if_any() const
{
    return _imp->required_confirmations;
}

void
ChangesToMakeDecision::add_required_confirmation(const std::tr1::shared_ptr<const RequiredConfirmation> & r)
{
    if (! _imp->required_confirmations)
        _imp->required_confirmations.reset(new RequiredConfirmations);
    _imp->required_confirmations->push_back(r);
}

const std::tr1::shared_ptr<const PackageID>
ChangesToMakeDecision::origin_id() const
{
    return _imp->origin_id;
}

ChangeType
ChangesToMakeDecision::change_type() const
{
    return _imp->change_type;
}

void
ChangesToMakeDecision::set_change_type(const ChangeType t)
{
    _imp->change_type = t;
}

bool
ChangesToMakeDecision::best() const
{
    return _imp->best;
}

const Resolvent
ChangesToMakeDecision::resolvent() const
{
    return _imp->resolvent;
}

bool
ChangesToMakeDecision::taken() const
{
    return _imp->taken;
}

const std::tr1::shared_ptr<const RepositoryName>
ChangesToMakeDecision::if_via_new_binary_in() const
{
    return _imp->if_via_new_binary_in;
}

void
ChangesToMakeDecision::set_via_new_binary_in(const RepositoryName & n)
{
    _imp->if_via_new_binary_in = make_shared_copy(n);
}

void
ChangesToMakeDecision::serialise(Serialiser & s) const
{
    s.object("ChangesToMakeDecision")
        .member(SerialiserFlags<>(), "resolvent", resolvent())
        .member(SerialiserFlags<serialise::might_be_null>(), "origin_id", origin_id())
        .member(SerialiserFlags<>(), "best", best())
        .member(SerialiserFlags<>(), "change_type", stringify(change_type()))
        .member(SerialiserFlags<serialise::might_be_null>(), "destination", destination())
        .member(SerialiserFlags<serialise::might_be_null>(), "if_via_new_binary_in", if_via_new_binary_in() ?
                make_shared_copy(stringify(*if_via_new_binary_in())) : make_null_shared_ptr())
        .member(SerialiserFlags<>(), "taken", taken())
        .member(SerialiserFlags<serialise::might_be_null, serialise::container>(), "required_confirmations_if_any", required_confirmations_if_any())
        ;
}

namespace paludis
{
    template <>
    struct Implementation<UnableToMakeDecision>
    {
        const Resolvent resolvent;
        const std::tr1::shared_ptr<const UnsuitableCandidates> unsuitable_candidates;
        const bool taken;

        Implementation(const Resolvent & l,
                const std::tr1::shared_ptr<const UnsuitableCandidates> & u, const bool t) :
            resolvent(l),
            unsuitable_candidates(u),
            taken(t)
        {
        }
    };
}

UnableToMakeDecision::UnableToMakeDecision(
        const Resolvent & l,
        const std::tr1::shared_ptr<const UnsuitableCandidates> & u,
        const bool t) :
    PrivateImplementationPattern<UnableToMakeDecision>(new Implementation<UnableToMakeDecision>(l, u, t))
{
}

UnableToMakeDecision::~UnableToMakeDecision() = default;

const std::tr1::shared_ptr<const UnsuitableCandidates>
UnableToMakeDecision::unsuitable_candidates() const
{
    return _imp->unsuitable_candidates;
}

const Resolvent
UnableToMakeDecision::resolvent() const
{
    return _imp->resolvent;
}

bool
UnableToMakeDecision::taken() const
{
    return _imp->taken;
}

void
UnableToMakeDecision::serialise(Serialiser & s) const
{
    s.object("UnableToMakeDecision")
        .member(SerialiserFlags<>(), "resolvent", resolvent())
        .member(SerialiserFlags<>(), "taken", taken())
        .member(SerialiserFlags<serialise::might_be_null, serialise::container>(), "unsuitable_candidates", unsuitable_candidates())
        ;
}

namespace paludis
{
    template <>
    struct Implementation<RemoveDecision>
    {
        const Resolvent resolvent;
        const std::tr1::shared_ptr<const PackageIDSequence> ids;
        const bool taken;
        std::tr1::shared_ptr<RequiredConfirmations> required_confirmations;

        Implementation(const Resolvent & l, const std::tr1::shared_ptr<const PackageIDSequence> & i, const bool t) :
            resolvent(l),
            ids(i),
            taken(t)
        {
        }
    };
}

RemoveDecision::RemoveDecision(const Resolvent & l, const std::tr1::shared_ptr<const PackageIDSequence> & i, const bool t) :
    PrivateImplementationPattern<RemoveDecision>(new Implementation<RemoveDecision>(l, i, t))
{
}

RemoveDecision::~RemoveDecision() = default;

const Resolvent
RemoveDecision::resolvent() const
{
    return _imp->resolvent;
}

bool
RemoveDecision::taken() const
{
    return _imp->taken;
}

const std::tr1::shared_ptr<const PackageIDSequence>
RemoveDecision::ids() const
{
    return _imp->ids;
}

const std::tr1::shared_ptr<const RequiredConfirmations>
RemoveDecision::required_confirmations_if_any() const
{
    return _imp->required_confirmations;
}

void
RemoveDecision::add_required_confirmation(const std::tr1::shared_ptr<const RequiredConfirmation> & r)
{
    if (! _imp->required_confirmations)
        _imp->required_confirmations.reset(new RequiredConfirmations);
    _imp->required_confirmations->push_back(r);
}

void
RemoveDecision::serialise(Serialiser & s) const
{
    s.object("RemoveDecision")
        .member(SerialiserFlags<>(), "resolvent", resolvent())
        .member(SerialiserFlags<>(), "taken", taken())
        .member(SerialiserFlags<serialise::might_be_null, serialise::container>(), "ids", ids())
        .member(SerialiserFlags<serialise::might_be_null, serialise::container>(), "required_confirmations_if_any", required_confirmations_if_any())
        ;
}

namespace paludis
{
    template <>
    struct Implementation<BreakDecision>
    {
        const Resolvent resolvent;
        const std::tr1::shared_ptr<const PackageID> existing_id;
        const bool taken;
        std::tr1::shared_ptr<RequiredConfirmations> required_confirmations;

        Implementation(const Resolvent & l,
                const std::tr1::shared_ptr<const PackageID> & e,
                const bool t) :
            resolvent(l),
            existing_id(e),
            taken(t)
        {
        }
    };
}

BreakDecision::BreakDecision(const Resolvent & l, const std::tr1::shared_ptr<const PackageID> & e, const bool t) :
    PrivateImplementationPattern<BreakDecision>(new Implementation<BreakDecision>(
                l, e, t))
{
}

BreakDecision::~BreakDecision() = default;

const std::tr1::shared_ptr<const PackageID>
BreakDecision::existing_id() const
{
    return _imp->existing_id;
}

const Resolvent
BreakDecision::resolvent() const
{
    return _imp->resolvent;
}

bool
BreakDecision::taken() const
{
    return _imp->taken;
}

const std::tr1::shared_ptr<const RequiredConfirmations>
BreakDecision::required_confirmations_if_any() const
{
    return _imp->required_confirmations;
}

void
BreakDecision::add_required_confirmation(const std::tr1::shared_ptr<const RequiredConfirmation> & r)
{
    if (! _imp->required_confirmations)
        _imp->required_confirmations.reset(new RequiredConfirmations);
    _imp->required_confirmations->push_back(r);
}

void
BreakDecision::serialise(Serialiser & s) const
{
    s.object("BreakDecision")
        .member(SerialiserFlags<serialise::might_be_null>(), "existing_id", existing_id())
        .member(SerialiserFlags<>(), "taken", taken())
        .member(SerialiserFlags<serialise::might_be_null, serialise::container>(), "required_confirmations_if_any", required_confirmations_if_any())
        ;
}

const std::tr1::shared_ptr<BreakDecision>
BreakDecision::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "BreakDecision");
    std::tr1::shared_ptr<BreakDecision> result(new BreakDecision(
                v.member<Resolvent>("resolvent"),
                v.member<std::tr1::shared_ptr<const PackageID> >("existing_id"),
                v.member<bool>("taken")
                ));

    {
        const std::tr1::shared_ptr<Deserialisation> dn(v.find_remove_member("required_confirmations_if_any"));
        if (! dn->null())
        {
            Deserialisator vv(*dn, "c");
            for (int n(1), n_end(vv.member<int>("count") + 1) ; n != n_end ; ++n)
                result->add_required_confirmation(vv.member<std::tr1::shared_ptr<RequiredConfirmation> >(stringify(n)));
        }
    }

    return result;
}

template class PrivateImplementationPattern<NothingNoChangeDecision>;
template class PrivateImplementationPattern<ExistingNoChangeDecision>;
template class PrivateImplementationPattern<ChangesToMakeDecision>;
template class PrivateImplementationPattern<UnableToMakeDecision>;
template class PrivateImplementationPattern<RemoveDecision>;
template class PrivateImplementationPattern<BreakDecision>;

