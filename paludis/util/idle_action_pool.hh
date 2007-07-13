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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_IDLE_ACTION_POOL_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_IDLE_ACTION_POOL_HH 1

#include <paludis/util/idle_action_pool-fwd.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/tr1_functional.hh>

namespace paludis
{
    class PALUDIS_VISIBLE IdleActionPool :
        public InstantiationPolicy<IdleActionPool, instantiation_method::SingletonTag>,
        private PrivateImplementationPattern<IdleActionPool>
    {
        friend class InstantiationPolicy<IdleActionPool, instantiation_method::SingletonTag>;

        private:
            IdleActionPool();
            ~IdleActionPool();

            void _count_result(const tr1::function<IdleActionResult () throw ()> &);

        public:
            void required_idle_action(const tr1::function<IdleActionResult () throw ()> &);
            void optional_idle_action(const tr1::function<IdleActionResult () throw ()> &);

            void increase_used_stat();
            void increase_unprepared_stat();
    };
}

#endif
