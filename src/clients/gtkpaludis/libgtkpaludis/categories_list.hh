/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef GTKPALUDIS_GUARD_LIBGTKPALUDIS_CATEGORIES_LIST_HH
#define GTKPALUDIS_GUARD_LIBGTKPALUDIS_CATEGORIES_LIST_HH 1

#include <paludis/util/private_implementation_pattern.hh>
#include <gtkmm/treeview.h>

namespace gtkpaludis
{
    class MainWindow;
    class PackagesPage;

    class CategoriesList :
        public Gtk::TreeView,
        private paludis::PrivateImplementationPattern<CategoriesList>
    {
        protected:
            void handle_signal_cursor_changed();

        public:
            CategoriesList(MainWindow * const m, PackagesPage * const p);
            ~CategoriesList();

            void populate();
    };
}

#endif
