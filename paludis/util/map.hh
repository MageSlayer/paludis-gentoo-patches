/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_MAP_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_MAP_HH 1

#include <paludis/util/map-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/private_implementation_pattern.hh>

#include <libwrapiter/libwrapiter_forward_iterator-fwd.hh>
#include <libwrapiter/libwrapiter_output_iterator-fwd.hh>

#include <utility>

namespace paludis
{
    template <typename K_, typename V_>
    class PALUDIS_VISIBLE Map :
        private PrivateImplementationPattern<Map<K_, V_> >,
        private InstantiationPolicy<Map<K_, V_>, instantiation_method::NonCopyableTag>
    {
        private:
            using PrivateImplementationPattern<Map<K_, V_> >::_imp;

        public:
            Map();
            ~Map();

            typedef libwrapiter::ForwardIterator<Map<K_, V_>, const std::pair<const K_, V_> > Iterator;
            Iterator begin() const PALUDIS_ATTRIBUTE((warn_unused_result));
            Iterator end() const PALUDIS_ATTRIBUTE((warn_unused_result));
            Iterator find(const K_ &) const PALUDIS_ATTRIBUTE((warn_unused_result));

            typedef libwrapiter::OutputIterator<Map<K_, V_>, std::pair<const K_, V_> > Inserter;
            Inserter inserter() PALUDIS_ATTRIBUTE((warn_unused_result));

            bool empty() const PALUDIS_ATTRIBUTE((warn_unused_result));
            unsigned size() const PALUDIS_ATTRIBUTE((warn_unused_result));

            void insert(const K_ &, const V_ &);
            void erase(const Iterator &);
            void erase(const K_ &);
    };
}

#endif
