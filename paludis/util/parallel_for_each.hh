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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_PARALLEL_FOR_EACH_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_PARALLEL_FOR_EACH_HH 1

#include <paludis/util/future.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/tr1_memory.hh>

#ifndef PALUDIS_ENABLE_THREADS
#  include <algorithm>
#endif

namespace paludis
{
    template <typename I_, typename P_>
    void parallel_for_each_worker(I_ cur, const I_ & end, const unsigned partition_size, const P_ & op)
    {
        while (cur != end)
        {
            op(*cur);

            for (unsigned x(0) ; x < partition_size ; ++x, ++cur)
                if (cur == end)
                    break;
        }
    }

    template <typename I_, typename P_>
    void parallel_for_each(I_ cur, const I_ & end, const P_ & op,
            const unsigned partition_size = FutureActionQueue::get_instance()->number_of_threads())
    {
#ifdef PALUDIS_ENABLE_THREADS
        Sequence<tr1::shared_ptr<Future<void> > > futures;
        for (unsigned x(0) ; x < partition_size ; ++x, ++cur)
        {
            if (cur == end)
                break;

            futures.push_back(make_shared_ptr(new Future<void>(
                            tr1::bind(parallel_for_each_worker<I_, P_>, I_(cur), tr1::cref(end), partition_size, tr1::cref(op)))));
        }
#else
        std::for_each(cur, end, op);
#endif
    }
}

#endif
