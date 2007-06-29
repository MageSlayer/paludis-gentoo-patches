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
#  include <paludis/util/thread.hh>
#  include <list>
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
        std::list<tr1::function<void () throw ()> > queue;
        Thread thread;

        void finish()
        {
            Lock l(mutex);
            should_finish = true;
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

        Implementation() :
            should_finish(false),
            thread(tr1::bind(tr1::mem_fn(&Implementation::thread_func), this))
        {
        }
#endif
    };
}

ActionQueue::ActionQueue() :
    PrivateImplementationPattern<ActionQueue>(new Implementation<ActionQueue>)
{
}

ActionQueue::~ActionQueue()
{
    enqueue(tr1::bind(tr1::mem_fn(&Implementation<ActionQueue>::finish), _imp.get()));
}

void
ActionQueue::enqueue(const tr1::function<void () throw ()> & f)
{
#ifdef PALUDIS_ENABLE_THREADS
    Lock l(_imp->mutex);
    _imp->queue.push_back(f);
    _imp->condition.signal();
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

