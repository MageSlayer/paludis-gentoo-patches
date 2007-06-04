/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef GTKPALUDIS_GUARD_LIBGTKPALUDIS_PACKAGES_LIST_HH
#define GTKPALUDIS_GUARD_LIBGTKPALUDIS_PACKAGES_LIST_HH 1

#include <paludis/util/private_implementation_pattern.hh>
#include <gtkmm/treeview.h>

namespace gtkpaludis
{
    class MainWindow;
    class PackagesPage;

    class PackagesList :
        public Gtk::TreeView,
        private paludis::PrivateImplementationPattern<PackagesList>
    {
        protected:
            void handle_signal_cursor_changed();
            void populate_in_paludis_thread();
            void populate_in_gui_thread();

        public:
            PackagesList(MainWindow * const m, PackagesPage * const p);
            ~PackagesList();

            void populate_real();
            void populate_filter();
    };
}


#endif
