/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "threaded_window.hh"
#include <paludis/util/exception.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <gtkmm/dialog.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/stock.h>
#include <tr1/functional>
#include <unistd.h>
#include <algorithm>
#include <list>

using namespace paludis;
using namespace gtkpaludis;

namespace paludis
{
    template<>
    struct Implementation<ThreadedWindow>
    {
        Environment * const environment;

        int sensitivity;
        Gtk::Widget * saved_focus;

        Glib::Mutex gui_queue_lock;
        std::list<sigc::slot<void> > gui_queue;

        Glib::Mutex paludis_queue_lock;
        std::list<sigc::slot<void> > paludis_queue;
        int paludis_queue_pipe[2];

        bool paludis_queue_exit;

        Glib::Thread * paludis_thread;

        Implementation(Environment * const e) :
            environment(e),
            sensitivity(0),
            saved_focus(0),
            paludis_queue_exit(false)
        {
            if (0 != pipe(paludis_queue_pipe))
                throw InternalError(PALUDIS_HERE, "pipe() failed");
        }
    };
}

ThreadedWindow::ThreadedWindow(Gtk::WindowType w, Environment * const e) :
    Gtk::Window(w),
    PrivateImplementationPattern<ThreadedWindow>(new Implementation<ThreadedWindow>(e))
{
    Glib::signal_timeout().connect(sigc::mem_fun(this, &ThreadedWindow::run_gui_queue), 10);
    _imp->paludis_thread = Glib::Thread::create(sigc::mem_fun(this, &ThreadedWindow::run_paludis_queue), true);
    signal_delete_event().connect(sigc::mem_fun(this, &ThreadedWindow::handle_delete_event));
}

ThreadedWindow::~ThreadedWindow()
{
}

bool
ThreadedWindow::handle_delete_event(GdkEventAny *)
{
    desensitise();

    _imp->paludis_queue_exit = true;
    char x('x');
    write(_imp->paludis_queue_pipe[1], &x, 1);
    _imp->paludis_thread->join();

    return false;
}

void
ThreadedWindow::paludis_thread_action(const sigc::slot<void> & a, const std::string & status_message)
{
    desensitise();
    Glib::Mutex::Lock l(_imp->paludis_queue_lock);
    _imp->paludis_queue.push_back(sigc::bind(sigc::mem_fun(this, &ThreadedWindow::paludis_thread_action_then_sensitise),
                a, status_message));

    char x('x');
    write(_imp->paludis_queue_pipe[1], &x, 1);
}

void
ThreadedWindow::paludis_thread_action_then_sensitise(const sigc::slot<void> & a, const std::string & status_message)
{
    if (! status_message.empty())
        gui_thread_action(sigc::bind(sigc::mem_fun(this, &ThreadedWindow::push_status_message), status_message));

    try
    {
        a();
    }
    catch (const Exception & e)
    {
        gui_thread_action(sigc::bind(sigc::mem_fun(this, &ThreadedWindow::notify_exception),
                    std::string(e.message()), std::string(e.what()), std::string(e.backtrace("\n"))));
    }
    catch (const std::exception & e)
    {
        gui_thread_action(sigc::bind(sigc::mem_fun(this, &ThreadedWindow::notify_exception),
                    std::string(""), std::string(e.what()), std::string("")));
    }

    if (! status_message.empty())
        gui_thread_action(sigc::mem_fun(this, &ThreadedWindow::pop_status_message));

    gui_thread_action(sigc::mem_fun(this, &ThreadedWindow::sensitise));
}

void
ThreadedWindow::gui_thread_action(const sigc::slot<void> & a)
{
    Glib::Mutex::Lock l(_imp->gui_queue_lock);
    _imp->gui_queue.push_back(a);
}

void
ThreadedWindow::sensitise()
{
    if (0 == --_imp->sensitivity)
    {
        do_set_sensitive(true);
        if (_imp->saved_focus)
        {
            set_focus(*_imp->saved_focus);
            _imp->saved_focus = 0;
        }
    }
}

void
ThreadedWindow::desensitise()
{
    if (0 == _imp->sensitivity++)
    {
        _imp->saved_focus = get_focus();
        do_set_sensitive(false);
    }
}

bool
ThreadedWindow::run_gui_queue()
{
    std::list<sigc::slot<void> > local_queue;

    {
        Glib::Mutex::Lock l(_imp->gui_queue_lock);
        if (! _imp->gui_queue.empty())
            local_queue.splice(local_queue.begin(), _imp->gui_queue, _imp->gui_queue.begin(), _imp->gui_queue.end());
    }

    std::for_each(local_queue.begin(), local_queue.end(), std::tr1::mem_fn(&sigc::slot<void>::operator ()));

    return true;
}

void
ThreadedWindow::run_paludis_queue()
{
    while (! _imp->paludis_queue_exit)
    {
        std::list<sigc::slot<void> > local_queue;

        {
            Glib::Mutex::Lock l(_imp->paludis_queue_lock);
            if (! _imp->paludis_queue.empty())
                local_queue.splice(local_queue.begin(), _imp->paludis_queue, _imp->paludis_queue.begin(), _imp->paludis_queue.end());
        }

        if (local_queue.empty())
        {
            char buf[1024];
            read(_imp->paludis_queue_pipe[0], &buf, 1023);
        }
        else
            std::for_each(local_queue.begin(), local_queue.end(), std::tr1::mem_fn(&sigc::slot<void>::operator ()));
    }
}

Environment *
ThreadedWindow::environment()
{
    return _imp->environment;
}

void
ThreadedWindow::do_set_sensitive(const bool v)
{
    set_sensitive(v);
}

void
ThreadedWindow::notify_exception(const std::string & message, const std::string & what,
        const std::string & backtrace)
{
    Gtk::MessageDialog dialog(*this, "Caught exception", false, Gtk::MESSAGE_ERROR,
            Gtk::BUTTONS_OK);
    dialog.set_title("Caught exception '" + what + "'");
    dialog.set_secondary_text(backtrace + "Caught exception '" + message + "' (" + what + ")");
    dialog.run();
}

