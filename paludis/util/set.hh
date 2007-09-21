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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_SET_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_SET_HH 1

#include <paludis/util/set-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/instantiation_policy.hh>

#include <libwrapiter/libwrapiter_forward_iterator-fwd.hh>
#include <libwrapiter/libwrapiter_output_iterator-fwd.hh>

namespace paludis
{
    template <typename T_, typename C_>
    class PALUDIS_VISIBLE Set :
        private PrivateImplementationPattern<Set<T_, C_> >,
        private InstantiationPolicy<Set<T_, C_>, instantiation_method::NonCopyableTag>
    {
        private:
            using PrivateImplementationPattern<Set<T_, C_> >::_imp;

        public:
            typedef T_ value_type;
            typedef T_ & reference;
            typedef const T_ & const_reference;

            Set();
            ~Set();

            typedef libwrapiter::ForwardIterator<Set<T_, C_>, const T_> Iterator;
            Iterator begin() const PALUDIS_ATTRIBUTE((warn_unused_result));
            Iterator end() const PALUDIS_ATTRIBUTE((warn_unused_result));
            Iterator find(const T_ &) const PALUDIS_ATTRIBUTE((warn_unused_result));

            bool empty() const PALUDIS_ATTRIBUTE((warn_unused_result));
            unsigned size() const PALUDIS_ATTRIBUTE((warn_unused_result));
            unsigned count(const T_ &) const PALUDIS_ATTRIBUTE((warn_unused_result));

            typedef libwrapiter::OutputIterator<Set<T_, C_>, T_> Inserter;
            Inserter inserter();

            void insert(const T_ &);
            void erase(const T_ &);
    };
}

#endif
