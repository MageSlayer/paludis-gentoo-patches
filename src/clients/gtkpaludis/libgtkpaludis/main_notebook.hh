/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef GTKPALUDIS_GUARD_GTKPALUDIS_LIBGTKPALUDIS_MAIN_NOTEBOOK_HH
#define GTKPALUDIS_GUARD_GTKPALUDIS_LIBGTKPALUDIS_MAIN_NOTEBOOK_HH 1

#include <gtkmm/notebook.h>
#include <paludis/util/private_implementation_pattern.hh>

namespace gtkpaludis
{
    class MainWindow;

    class MainNotebook :
        public Gtk::Notebook,
        private paludis::PrivateImplementationPattern<MainNotebook>
    {
        protected:
            void handle_switch_page(GtkNotebookPage *, guint);

        public:
            MainNotebook(MainWindow * const);
            ~MainNotebook();

            void populate();

            void mark_messages_page();
            void unmark_messages_page();
    };
}

#endif
