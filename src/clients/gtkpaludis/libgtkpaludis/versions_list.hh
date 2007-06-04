/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef GTKPALUDIS_GUARD_LIBGTKPALUDIS_VERSIONS_LIST_HH
#define GTKPALUDIS_GUARD_LIBGTKPALUDIS_VERSIONS_LIST_HH 1

#include <paludis/util/private_implementation_pattern.hh>
#include <gtkmm/treeview.h>

namespace gtkpaludis
{
    class QueryWindow;
    class VersionsPage;

    class VersionsList :
        public Gtk::TreeView,
        private paludis::PrivateImplementationPattern<VersionsList>
    {
        protected:
            void handle_signal_cursor_changed();
            void populate_in_paludis_thread();
            void populate_in_gui_thread();

        public:
            VersionsList(QueryWindow * const m, VersionsPage * const p);
            ~VersionsList();

            void populate();
    };
}

#endif
