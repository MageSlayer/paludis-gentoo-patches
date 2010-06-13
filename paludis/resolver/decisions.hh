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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_DECISIONS_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_DECISIONS_HH 1

#include <paludis/resolver/decisions-fwd.hh>
#include <paludis/resolver/decision-fwd.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <tr1/memory>

namespace paludis
{
    namespace resolver
    {
        template <typename Decision_>
        struct DecisionsConstIteratorTag;

        template <typename Decision_>
        class PALUDIS_VISIBLE Decisions :
            private PrivateImplementationPattern<Decisions<Decision_> >
        {
            using PrivateImplementationPattern<Decisions<Decision_> >::_imp;

            public:
                Decisions();
                ~Decisions();

                void push_back(const std::tr1::shared_ptr<const Decision_> &);
                void cast_push_back(const std::tr1::shared_ptr<const Decision> &);

                typedef DecisionsConstIteratorTag<Decision_> ConstIteratorTag;
                typedef WrappedForwardIterator<ConstIteratorTag, const std::tr1::shared_ptr<const Decision_> > ConstIterator;
                ConstIterator begin() const PALUDIS_ATTRIBUTE((warn_unused_result));
                ConstIterator end() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        extern template class Decisions<UnableToMakeDecision>;
        extern template class Decisions<ChangesToMakeDecision>;
        extern template class Decisions<ChangeOrRemoveDecision>;
    }

    extern template class WrappedForwardIterator<resolver::DecisionsConstIteratorTag<resolver::UnableToMakeDecision>,
           const std::tr1::shared_ptr<const resolver::UnableToMakeDecision> >;
    extern template class WrappedForwardIterator<resolver::DecisionsConstIteratorTag<resolver::ChangesToMakeDecision>,
           const std::tr1::shared_ptr<const resolver::ChangesToMakeDecision> >;
    extern template class WrappedForwardIterator<resolver::DecisionsConstIteratorTag<resolver::ChangeOrRemoveDecision>,
           const std::tr1::shared_ptr<const resolver::ChangeOrRemoveDecision> >;
}

#endif
