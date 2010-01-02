/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009 Ciaran McCreesh
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

#include <paludis/util/executor.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/condition_variable.hh>
#include <paludis/util/thread.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <map>
#include <list>

using namespace paludis;

typedef std::multimap<std::string, std::tr1::shared_ptr<Executive> > Queues;
typedef std::list<std::tr1::shared_ptr<Executive> > ReadyForPost;

Executive::~Executive()
{
}

namespace paludis
{
    template <>
    struct Implementation<Executor>
    {
        int pending;
        int active;
        int done;

        Queues queues;
        ReadyForPost ready_for_post;
        Mutex mutex;
        ConditionVariable condition;

        Implementation() :
            pending(0),
            active(0),
            done(0)
        {
        }
    };
}

Executor::Executor() :
    PrivateImplementationPattern<Executor>(new Implementation<Executor>)
{
}

Executor::~Executor()
{
}

void
Executor::_one(const std::tr1::shared_ptr<Executive> executive)
{
    executive->execute_threaded();

    Lock lock(_imp->mutex);
    _imp->ready_for_post.push_back(executive);
    _imp->condition.signal();
}


int
Executor::pending() const
{
    return _imp->pending;
}

int
Executor::active() const
{
    return _imp->active;
}

int
Executor::done() const
{
    return _imp->done;
}

void
Executor::add(const std::tr1::shared_ptr<Executive> & x)
{
    ++_imp->pending;
    _imp->queues.insert(std::make_pair(x->queue_name(), x));
}

void
Executor::execute()
{
    typedef std::map<std::string, std::pair<std::tr1::shared_ptr<Thread>, std::tr1::shared_ptr<Executive> > > Running;
    Running running;

    Lock lock(_imp->mutex);
    while (true)
    {
        bool any(false);
        for (Queues::iterator q(_imp->queues.begin()), q_end(_imp->queues.end()) ;
                q != q_end ; )
        {
            if ((running.end() != running.find(q->first)) || ! q->second->can_run())
            {
                ++q;
                continue;
            }

            ++_imp->active;
            --_imp->pending;
            q->second->pre_execute_exclusive();
            running.insert(std::make_pair(q->first, std::make_pair(make_shared_ptr(new Thread(
                                std::tr1::bind(&Executor::_one, this, q->second))), q->second)));
            _imp->queues.erase(q++);
            any = true;
        }

        if ((! any) && running.empty())
            break;

        _imp->condition.timed_wait(_imp->mutex, 1);

        for (Running::iterator r(running.begin()), r_end(running.end()) ;
                r != r_end ; ++r)
            r->second.second->flush_threaded();

        for (ReadyForPost::iterator p(_imp->ready_for_post.begin()), p_end(_imp->ready_for_post.end()) ;
                p != p_end ; ++p)
        {
            --_imp->active;
            ++_imp->done;
            running.erase((*p)->queue_name());
            (*p)->post_execute_exclusive();
        }

        _imp->ready_for_post.clear();
    }
}

template class PrivateImplementationPattern<Executor>;

