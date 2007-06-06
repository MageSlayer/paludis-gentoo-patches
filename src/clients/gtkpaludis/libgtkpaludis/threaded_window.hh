/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef GTKPALUDIS_GUARD_LIBGTKPALUDIS_THREADED_WINDOW_HH
#define GTKPALUDIS_GUARD_LIBGTKPALUDIS_THREADED_WINDOW_HH 1

#include <gtkmm/window.h>
#include <paludis/util/private_implementation_pattern.hh>

namespace paludis
{
    class Environment;
}

namespace gtkpaludis
{
    class ThreadedWindow :
        public Gtk::Window,
        private paludis::PrivateImplementationPattern<ThreadedWindow>
    {
        protected:
            virtual bool handle_delete_event(GdkEventAny *);

            void paludis_thread_action_then_sensitise(const sigc::slot<void> &, const std::string & status_message);
            void desensitise();
            void sensitise();
            virtual void do_set_sensitive(const bool);

            bool run_gui_queue();
            void run_paludis_queue();

            ThreadedWindow(Gtk::WindowType, paludis::Environment * const);

            virtual void push_status_message(const std::string &) = 0;
            virtual void pop_status_message() = 0;

        public:
            ~ThreadedWindow();

            void paludis_thread_action(const sigc::slot<void> &, const std::string & status_message);
            void gui_thread_action(const sigc::slot<void> &);

            virtual void notify_exception(const std::string & message, const std::string & what,
                    const std::string & backtrace);

            paludis::Environment * environment();
    };
}

#endif
