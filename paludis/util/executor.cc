/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010, 2011, 2012, 2013 Ciaran McCreesh
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
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/stringify.hh>
#include <map>
#include <list>
#include <thread>
#include <mutex>
#include <condition_variable>

using namespace paludis;

typedef std::list<std::shared_ptr<Executive> > ExecutiveList;
typedef std::map<std::string, ExecutiveList> Queues;
typedef std::list<std::shared_ptr<Executive> > ReadyForPost;

Executive::~Executive()
{
}

namespace paludis
{
    template <>
    struct Imp<Executor>
    {
        int ms_update_interval;
        int pending;
        int active;
        int done;

        Queues queues;
        ReadyForPost ready_for_post;
        std::mutex mutex;
        std::condition_variable condition;

        Imp(int u) :
            ms_update_interval(u),
            pending(0),
            active(0),
            done(0)
        {
        }
    };
}

Executor::Executor(int ms_update_interval) :
    _imp(ms_update_interval)
{
}

Executor::~Executor()
{
}

void
Executor::_one(const std::shared_ptr<Executive> executive)
{
    executive->execute_threaded();

    std::unique_lock<std::mutex> lock(_imp->mutex);
    _imp->ready_for_post.push_back(executive);
    _imp->condition.notify_all();
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
Executor::add(const std::shared_ptr<Executive> & x)
{
    ++_imp->pending;
    _imp->queues.insert(std::make_pair(x->queue_name(), ExecutiveList())).first->second.push_back(x);
}

void
Executor::execute()
{
    typedef std::map<std::string, std::pair<std::thread, std::shared_ptr<Executive> > > Running;
    Running running;

    std::unique_lock<std::mutex> lock(_imp->mutex);
    while (true)
    {
        bool any(false);
        for (Queues::iterator q(_imp->queues.begin()), q_end(_imp->queues.end()) ;
                q != q_end ; )
        {
            if ((running.end() != running.find(q->first)) || ! (*q->second.begin())->can_run())
            {
                ++q;
                continue;
            }

            ++_imp->active;
            --_imp->pending;
            (*q->second.begin())->pre_execute_exclusive();
            running.insert(std::make_pair(q->first, std::make_pair(std::thread(std::bind(&Executor::_one, this, *q->second.begin())), *q->second.begin())));
            q->second.erase(q->second.begin());
            if (q->second.empty())
                _imp->queues.erase(q++);
            else
                ++q;
            any = true;
        }

        if ((! any) && running.empty())
        {
            if (! _imp->queues.empty())
                throw InternalError(PALUDIS_HERE, "None of our executives can start, but queues are not empty");
            break;
        }

        _imp->condition.wait_for(lock, std::chrono::milliseconds(_imp->ms_update_interval));

        for (Running::iterator r(running.begin()), r_end(running.end()) ;
                r != r_end ; ++r)
            r->second.second->flush_threaded();

        for (ReadyForPost::iterator p(_imp->ready_for_post.begin()), p_end(_imp->ready_for_post.end()) ;
                p != p_end ; ++p)
        {
            --_imp->active;
            ++_imp->done;
            auto r = running.find((*p)->queue_name());
            r->second.first.join();
            running.erase(r);
            (*p)->post_execute_exclusive();
        }

        _imp->ready_for_post.clear();
    }
}

std::mutex &
Executor::exclusivity_mutex()
{
    return _imp->mutex;
}

namespace paludis
{
    template class Pimp<Executor>;
}

