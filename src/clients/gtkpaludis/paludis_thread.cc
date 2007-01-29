/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include "paludis_thread.hh"
#include "main_window.hh"
#include <paludis/util/log.hh>
#include <paludis/util/stringify.hh>
#include <glibmm/thread.h>

using namespace paludis;
using namespace gtkpaludis;

namespace paludis
{
    template<>
    struct Implementation<PaludisThread>
    {
        Glib::Dispatcher dispatcher;
        Glib::Mutex queue_mutex, single_mutex;
        std::deque<sigc::slot<void> > * queue;

        Implementation() :
            queue(new std::deque<sigc::slot<void> >)
        {
        }

        ~Implementation()
        {
            delete queue;
        }
    };
}

PaludisThread::PaludisThread() :
    PrivateImplementationPattern<PaludisThread>(new Implementation<PaludisThread>)
{
    _imp->dispatcher.connect(sigc::mem_fun(this, &PaludisThread::_queue_run));
}

PaludisThread::~PaludisThread()
{
}

PaludisThread::Launchable::Launchable()
{
}

PaludisThread::Launchable::~Launchable()
{
}

void
PaludisThread::launch(std::tr1::shared_ptr<Launchable> l)
{
    MainWindow::get_instance()->lock_controls();
    Glib::Thread::create(sigc::bind<1>(sigc::mem_fun(this, &PaludisThread::_thread_func), l), false);
}

void
PaludisThread::Launchable::dispatch(const sigc::slot<void> & s)
{
    PaludisThread::get_instance()->_queue_add(s);
}

void
PaludisThread::_queue_add(const sigc::slot<void> & s)
{
    {
        Glib::Mutex::Lock lock(_imp->queue_mutex);
        _imp->queue->push_back(s);
    }
    _imp->dispatcher();
}

void
PaludisThread::_queue_run()
{
    std::deque<sigc::slot<void> > * d(new std::deque<sigc::slot<void> >);
    {
        /* do as little as possible inside the mutex lock */
        Glib::Mutex::Lock lock(_imp->queue_mutex);
        std::swap(d, _imp->queue);
    }

    std::for_each(d->begin(), d->end(), std::mem_fun_ref(&sigc::slot<void>::operator()));
    delete d;
}

void
PaludisThread::_thread_func(std::tr1::shared_ptr<PaludisThread::Launchable> l)
{
    {
        Glib::Mutex::Lock lock(_imp->single_mutex);
        try
        {
            try
            {
                (*l)();
            }
            catch (const Exception &)
            {
                throw;
            }
            catch (const std::exception & e)
            {
                throw InternalError(PALUDIS_HERE, "Caught unexpected exception '" + stringify(e.what()) + "'");
            }
        }
        catch (const InternalError & e)
        {
            _queue_add(sigc::bind<std::string, std::string, bool>(
                        sigc::mem_fun(MainWindow::get_instance(), &MainWindow::show_exception),
                        stringify(e.what()), e.message(), true));
        }
        catch (const Exception & e)
        {
            _queue_add(sigc::bind<std::string, std::string, bool>(
                        sigc::mem_fun(MainWindow::get_instance(), &MainWindow::show_exception),
                        stringify(e.what()), e.message(), false));
        }
    }
    _queue_add(sigc::mem_fun(MainWindow::get_instance(), &MainWindow::maybe_unlock_controls));
}

PaludisThread::Launchable::StatusBarMessage::StatusBarMessage(Launchable * const l, const std::string & s) :
    _l(l)
{
    _l->dispatch(sigc::bind<1>(sigc::mem_fun(MainWindow::get_instance(), &MainWindow::push_status), s));
}

PaludisThread::Launchable::StatusBarMessage::~StatusBarMessage()
{
    _l->dispatch(sigc::mem_fun(MainWindow::get_instance(), &MainWindow::pop_status));
}

bool
PaludisThread::try_lock_unlock()
{
    if (_imp->single_mutex.trylock())
    {
        _imp->single_mutex.unlock();
        return true;
    }
    return false;
}

