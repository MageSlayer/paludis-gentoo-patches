/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2010 Ciaran McCreesh
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

#include <paludis/util/action_queue.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/condition_variable.hh>
#include <paludis/util/thread_pool.hh>
#include <paludis/util/thread.hh>
#include <deque>

using namespace paludis;

template class PrivateImplementationPattern<ActionQueue>;

namespace paludis
{
    template <>
    struct Implementation<ActionQueue>
    {
        Mutex mutex;
        ConditionVariable condition;
        std::deque<std::function<void () throw ()> > queue;
        ThreadPool threads;
        bool limit_size;
        bool should_finish;

        void finish()
        {
            Lock l(mutex);
            should_finish = true;
            condition.broadcast();
        }

        void thread_func()
        {
            while (true)
            {
                std::function<void () throw ()> func;
                {
                    Lock l(mutex);
                    if (queue.empty())
                    {
                        if (should_finish)
                            break;

                        condition.wait(mutex);
                        continue;
                    }
                    else
                    {
                        func = *queue.begin();
                        queue.pop_front();
                    }
                }

                func();
            }
        }

        Implementation(const unsigned n_threads, const bool nice, const bool do_limit_size) :
            limit_size(do_limit_size),
            should_finish(false)
        {
            for (unsigned x(0) ; x < n_threads ; ++x)
                if (nice)
                    threads.create_thread(std::bind(&Thread::idle_adapter,
                                std::function<void () throw ()>(std::bind(std::mem_fn(&Implementation::thread_func), this))));
                else
                    threads.create_thread(std::bind(std::mem_fn(&Implementation::thread_func), this));
        }
    };
}

ActionQueue::ActionQueue(const unsigned n_threads, const bool nice, const bool limit_size) :
    PrivateImplementationPattern<ActionQueue>(n_threads, nice, limit_size)
{
}

ActionQueue::~ActionQueue()
{
    enqueue(std::bind(std::mem_fn(&Implementation<ActionQueue>::finish), _imp.get()));
}

void
ActionQueue::enqueue(const std::function<void () throw ()> & f)
{
    Lock l(_imp->mutex);
    if ((! _imp->limit_size) || (_imp->queue.size() < 1000))
    {
        _imp->queue.push_back(f);
        _imp->condition.signal();
    }
    else
        f();
}

void
ActionQueue::complete_pending()
{
    ConditionVariable c;
    Mutex m;
    Lock l(m);

    enqueue(std::bind(std::mem_fn(&ConditionVariable::acquire_then_signal), &c, std::ref(m)));
    c.wait(m);
}

void
ActionQueue::forget_pending()
{
    Lock l(_imp->mutex);
    _imp->queue.clear();
}

unsigned
ActionQueue::number_of_threads() const
{
    return _imp->threads.number_of_threads();
}

