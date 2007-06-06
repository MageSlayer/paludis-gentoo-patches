/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef PALUDIS_GUARD_SRC_CLIENTS_GTKPALUDIS_LIBGTKPALUDIS_TASK_WINDOW_HH
#define PALUDIS_GUARD_SRC_CLIENTS_GTKPALUDIS_LIBGTKPALUDIS_TASK_WINDOW_HH 1

#include <libgtkpaludis/threaded_window.hh>
#include <paludis/util/private_implementation_pattern.hh>

namespace gtkpaludis
{
    class MainWindow;
    class GuiTask;

    class TaskWindow :
        public ThreadedWindow,
        private paludis::PrivateImplementationPattern<TaskWindow>
    {
        private:
            paludis::PrivateImplementationPattern<TaskWindow>::ImpPtr & _imp;

        protected:
            virtual bool handle_delete_event(GdkEventAny *);
            void handle_ok_button_clicked();
            virtual void do_set_sensitive(const bool);

            virtual void push_status_message(const std::string &);
            virtual void pop_status_message();

        public:
            TaskWindow(MainWindow * const, GuiTask * const);
            ~TaskWindow();

            void run();

            void append_sequence_item(const std::string & id, const std::string & description,
                    const std::string & status);
            void set_sequence_item_status(const std::string & id, const std::string & status);
    };
}

#endif
