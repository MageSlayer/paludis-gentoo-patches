/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef GTKPALUDIS_GUARD_GTKPALUDIS_MAIN_WINDOW_HH
#define GTKPALUDIS_GUARD_GTKPALUDIS_MAIN_WINDOW_HH 1

#include <libgtkpaludis/threaded_window.hh>
#include <paludis/util/private_implementation_pattern.hh>

namespace gtkpaludis
{
    class MainNotebook;

    class MainWindow :
        public ThreadedWindow,
        private paludis::PrivateImplementationPattern<MainWindow>
    {
        private:
            paludis::PrivateImplementationPattern<MainWindow>::ImpPtr & _imp;

        protected:
            virtual void push_status_message(const std::string &);
            virtual void pop_status_message();

            virtual void do_set_sensitive(const bool);

        public:
            MainWindow(paludis::Environment * const);
            ~MainWindow();

            MainNotebook * main_notebook();

            using ThreadedWindow::sensitise;
            using ThreadedWindow::desensitise;

            void set_capture_output_options();
    };
}

#endif
