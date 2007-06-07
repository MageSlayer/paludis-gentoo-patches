/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef GTKPALUDIS_GUARD_LIBGTKPALUDIS_SETS_LIST_HH
#define GTKPALUDIS_GUARD_LIBGTKPALUDIS_SETS_LIST_HH 1

#include <paludis/util/private_implementation_pattern.hh>
#include <gtkmm/treeview.h>

namespace gtkpaludis
{
    class MainWindow;
    class PackagesPage;

    class SetsList :
        public Gtk::TreeView,
        private paludis::PrivateImplementationPattern<SetsList>
    {
        protected:
            void handle_signal_cursor_changed();

        public:
            SetsList(MainWindow * const m, PackagesPage * const p);
            ~SetsList();

            void populate();
    };
}


#endif
