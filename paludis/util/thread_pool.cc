/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2010, 2011, 2012 Ciaran McCreesh
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

#include <paludis/util/thread_pool.hh>
#include <paludis/util/pimp-impl.hh>
#include <memory>
#include <deque>
#include <thread>

using namespace paludis;

namespace paludis
{
    template <>
    struct Imp<ThreadPool>
    {
        std::deque<std::thread> threads;
    };
}

ThreadPool::ThreadPool() :
    _imp()
{
}

ThreadPool::~ThreadPool()
{
    for (auto & t : _imp->threads)
        t.join();
}

void
ThreadPool::create_thread(const std::function<void ()> & f)
{
    _imp->threads.emplace_back(f);
}

unsigned
ThreadPool::number_of_threads() const
{
    return _imp->threads.size();
}

namespace paludis
{
    template class Pimp<ThreadPool>;
}

