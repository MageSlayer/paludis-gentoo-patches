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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_FUTURE_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_FUTURE_HH 1

#include <paludis/util/attributes.hh>
#include <paludis/util/tr1_functional.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/action_queue.hh>

namespace paludis
{
    class PALUDIS_VISIBLE FutureActionQueue :
        public ActionQueue,
        public InstantiationPolicy<FutureActionQueue, instantiation_method::SingletonTag>
    {
        friend class InstantiationPolicy<FutureActionQueue, instantiation_method::SingletonTag>;

        public:
            FutureActionQueue();
            ~FutureActionQueue();
    };

    template <typename T_>
    class PALUDIS_VISIBLE Future :
        private PrivateImplementationPattern<Future<T_> >
    {
        private:
            using PrivateImplementationPattern<Future<T_> >::_imp;

        public:
            Future(const tr1::function<T_ () throw ()> &);
            ~Future();

            T_ operator() () const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    template <>
    class PALUDIS_VISIBLE Future<void> :
        private PrivateImplementationPattern<Future<void> >
    {
        private:
            using PrivateImplementationPattern<Future<void> >::_imp;

        public:
            Future(const tr1::function<void () throw ()> &);
            ~Future();

            void operator() () const;
    };
}

#endif
