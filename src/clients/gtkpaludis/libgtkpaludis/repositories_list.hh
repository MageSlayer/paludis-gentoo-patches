/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef GTKPALUDIS_GUARD_LIBGTKPALUDIS_REPOSITORIES_LIST_HH
#define GTKPALUDIS_GUARD_LIBGTKPALUDIS_REPOSITORIES_LIST_HH 1

#include <paludis/util/private_implementation_pattern.hh>
#include <gtkmm/treeview.h>

namespace gtkpaludis
{
    class MainWindow;
    class RepositoriesPage;

    class RepositoriesList :
        public Gtk::TreeView,
        private paludis::PrivateImplementationPattern<RepositoriesList>
    {
        protected:
            void handle_signal_cursor_changed();

        public:
            RepositoriesList(MainWindow * const m, RepositoriesPage * const p);
            ~RepositoriesList();

            void populate();
    };
}

#endif
