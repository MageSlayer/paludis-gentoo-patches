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

#include <paludis/util/action_queue.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>

#ifdef PALUDIS_ENABLE_THREADS
#  include <paludis/util/mutex.hh>
#  include <paludis/util/condition_variable.hh>
#  include <paludis/util/thread_pool.hh>
#  include <paludis/util/thread.hh>
#  include <deque>
#endif

using namespace paludis;

namespace paludis
{
    template <>
    struct Implementation<ActionQueue>
    {
#ifdef PALUDIS_ENABLE_THREADS
        Mutex mutex;
        ConditionVariable condition;
        bool should_finish;
        std::deque<tr1::function<void () throw ()> > queue;
        ThreadPool threads;

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
                tr1::function<void () throw ()> func;
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

        Implementation(const unsigned n_threads, const bool nice) :
            should_finish(false)
        {
            for (unsigned x(0) ; x < n_threads ; ++x)
                if (nice)
                    threads.create_thread(tr1::bind(&Thread::idle_adapter,
                                tr1::function<void () throw ()>(tr1::bind(tr1::mem_fn(&Implementation::thread_func), this))));
                else
                    threads.create_thread(tr1::bind(tr1::mem_fn(&Implementation::thread_func), this));
        }
#endif
    };
}

#ifdef PALUDIS_ENABLE_THREADS
ActionQueue::ActionQueue(const unsigned n_threads, const bool nice) :
    PrivateImplementationPattern<ActionQueue>(new Implementation<ActionQueue>(n_threads, nice))
{
}
#else
ActionQueue::ActionQueue(const unsigned, const bool) :
    PrivateImplementationPattern<ActionQueue>(new Implementation<ActionQueue>())
{
}
#endif

ActionQueue::~ActionQueue()
{
#ifdef PALUDIS_ENABLE_THREADS
    enqueue(tr1::bind(tr1::mem_fn(&Implementation<ActionQueue>::finish), _imp.get()));
#endif
}

void
ActionQueue::enqueue(const tr1::function<void () throw ()> & f)
{
#ifdef PALUDIS_ENABLE_THREADS
    Lock l(_imp->mutex);
    if (_imp->queue.size() < 1000)
    {
        _imp->queue.push_back(f);
        _imp->condition.signal();
    }
    else
        f();
#else
    f();
#endif
}

void
ActionQueue::complete_pending()
{
#ifdef PALUDIS_ENABLE_THREADS
    ConditionVariable c;
    Mutex m;
    Lock l(m);

    enqueue(tr1::bind(tr1::mem_fn(&ConditionVariable::acquire_then_signal), &c, tr1::ref(m)));
    c.wait(m);
#endif
}

void
ActionQueue::forget_pending()
{
#ifdef PALUDIS_ENABLE_THREADS
    Lock l(_imp->mutex);
    _imp->queue.clear();
#endif
}

unsigned
ActionQueue::number_of_threads() const
{
#ifdef PALUDIS_ENABLE_THREADS
    return _imp->threads.number_of_threads();
#else
    return 0;
#endif
}

