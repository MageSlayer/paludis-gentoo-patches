/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef GTKPALUDIS_GUARD_LIBGTKPALUDIS_MESSAGES_PAGE_HH
#define GTKPALUDIS_GUARD_LIBGTKPALUDIS_MESSAGES_PAGE_HH 1

#include <paludis/util/private_implementation_pattern.hh>
#include <libgtkpaludis/main_notebook_page.hh>
#include <gtkmm/table.h>

namespace gtkpaludis
{
    class MainWindow;
    class MainNotebook;

    class MessagesPage :
        public Gtk::Table,
        public MainNotebookPage,
        private paludis::PrivateImplementationPattern<MessagesPage>
    {
        protected:
            void handle_terminal_cursor_moved();

        public:
            MessagesPage(MainWindow * const m, MainNotebook * const n);
            ~MessagesPage();

            void populate();
    };
}


#endif
