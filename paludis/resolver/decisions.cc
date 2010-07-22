/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Ciaran McCreesh
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

#include <paludis/resolver/decisions.hh>
#include <paludis/resolver/decision.hh>
#include <paludis/resolver/orderer_notes.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/serialise-impl.hh>
#include <list>

using namespace paludis;
using namespace paludis::resolver;

namespace
{
    template <typename Decision_, typename Notes_>
    struct ContainerTraits
    {
        typedef std::list<std::pair<
            std::shared_ptr<const Decision_>,
            Notes_> > ContainerType;

        static void do_push_back(
                ContainerType & c,
                const std::shared_ptr<const Decision_> & d,
                const Notes_ & n)
        {
            c.push_back(std::make_pair(d, n));
        }

        static void do_member(SerialiserObjectWriter & w, int n,
                const std::pair<std::shared_ptr<const Decision_>, Notes_> & d)
        {
            w.member(SerialiserFlags<serialise::might_be_null>(), stringify(n) + ".1", d.first);
            w.member(SerialiserFlags<serialise::might_be_null>(), stringify(n) + ".2", d.second);
        }

        static void do_extract(
                std::shared_ptr<Decisions<Decision_, Notes_> > & result,
                Deserialisator & v,
                int n)
        {
            result->push_back(
                    v.member<std::shared_ptr<Decision_> >(stringify(n) + ".1"),
                    v.member<Notes_>(stringify(n) + ".2")
                    );
        }
    };

    template <typename Decision_>
    struct ContainerTraits<Decision_, NoType<0u> *>
    {
        typedef std::list<std::shared_ptr<const Decision_> > ContainerType;

        static void do_push_back(
                ContainerType & c,
                const std::shared_ptr<const Decision_> & d,
                const NoType<0u> * const)
        {
            c.push_back(d);
        }

        static void do_member(SerialiserObjectWriter & w, int n, const std::shared_ptr<const Decision_> & d)
        {
            w.member(SerialiserFlags<serialise::might_be_null>(), stringify(n), d);
        }

        static void do_extract(
                std::shared_ptr<Decisions<Decision_> > & result,
                Deserialisator & v,
                int n)
        {
            result->push_back(v.member<std::shared_ptr<Decision_> >(stringify(n)));
        }
    };
}

namespace paludis
{
    template <typename Decision_, typename Notes_>
    struct Implementation<Decisions<Decision_, Notes_> >
    {
        typename ContainerTraits<Decision_, Notes_>::ContainerType values;
    };

    template <typename Decision_, typename Notes_>
    struct WrappedForwardIteratorTraits<DecisionsConstIteratorTag<Decision_, Notes_> >
    {
        typedef typename ContainerTraits<Decision_, Notes_>::ContainerType::const_iterator UnderlyingIterator;
    };
}

template <typename Decision_, typename Notes_>
Decisions<Decision_, Notes_>::Decisions() :
    PrivateImplementationPattern<Decisions<Decision_, Notes_> >(new Implementation<Decisions<Decision_, Notes_> >)
{
}

template <typename Decision_, typename Notes_>
Decisions<Decision_, Notes_>::~Decisions()
{
}

template <typename Decision_, typename Notes_>
void
Decisions<Decision_, Notes_>::push_back(
        const std::shared_ptr<const Decision_> & d,
        const Notes_ & n)
{
    ContainerTraits<Decision_, Notes_>::do_push_back(_imp->values, d, n);
}

template <typename Decision_, typename Notes_>
typename Decisions<Decision_, Notes_>::ConstIterator
Decisions<Decision_, Notes_>::begin() const
{
    return ConstIterator(_imp->values.begin());
}

template <typename Decision_, typename Notes_>
typename Decisions<Decision_, Notes_>::ConstIterator
Decisions<Decision_, Notes_>::end() const
{
    return ConstIterator(_imp->values.end());
}

template <typename Decision_, typename Notes_>
bool
Decisions<Decision_, Notes_>::empty() const
{
    return _imp->values.empty();
}

template <typename Decision_, typename Notes_>
void
Decisions<Decision_, Notes_>::serialise(Serialiser & s) const
{
    SerialiserObjectWriter w(s.object("Decisions"));

    int n(0);
    for (ConstIterator i(begin()), i_end(end()) ;
            i != i_end ; ++i)
        ContainerTraits<Decision_, Notes_>::do_member(w, ++n, *i);

    w.member(SerialiserFlags<>(), "count", stringify(n));
}

template <typename Decision_, typename Notes_>
const std::shared_ptr<Decisions<Decision_, Notes_> >
Decisions<Decision_, Notes_>::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "Decisions");
    std::shared_ptr<Decisions<Decision_, Notes_> > result(new Decisions<Decision_, Notes_>);
    for (int n(1), n_end(v.member<int>("count") + 1) ; n != n_end ; ++n)
        ContainerTraits<Decision_, Notes_>::do_extract(result, v, n);
    return result;
}

template class Decisions<UnableToMakeDecision>;
template class Decisions<ChangesToMakeDecision>;
template class Decisions<ChangeOrRemoveDecision>;
template class Decisions<ConfirmableDecision>;
template class Decisions<ChangeOrRemoveDecision, std::shared_ptr<const OrdererNotes> >;

template class WrappedForwardIterator<Decisions<UnableToMakeDecision>::ConstIteratorTag,
         const std::shared_ptr<const UnableToMakeDecision> >;
template class WrappedForwardIterator<Decisions<ChangesToMakeDecision>::ConstIteratorTag,
         const std::shared_ptr<const ChangesToMakeDecision> >;
template class WrappedForwardIterator<Decisions<ChangeOrRemoveDecision>::ConstIteratorTag,
         const std::shared_ptr<const ChangeOrRemoveDecision> >;
template class WrappedForwardIterator<Decisions<ConfirmableDecision>::ConstIteratorTag,
         const std::shared_ptr<const ConfirmableDecision> >;
template class WrappedForwardIterator<Decisions<ChangeOrRemoveDecision, std::shared_ptr<const OrdererNotes> >::ConstIteratorTag,
         const std::pair<
             std::shared_ptr<const ChangeOrRemoveDecision>,
             std::shared_ptr<const OrdererNotes> > >;

