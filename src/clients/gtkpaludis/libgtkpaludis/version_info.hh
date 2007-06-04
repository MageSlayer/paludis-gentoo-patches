/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef GTKPALUDIS_GUARD_LIBGTKPALUDIS_VERSION_INFO_HH
#define GTKPALUDIS_GUARD_LIBGTKPALUDIS_VERSION_INFO_HH 1

#include <paludis/util/private_implementation_pattern.hh>
#include <gtkmm/treeview.h>

namespace gtkpaludis
{
    class QueryWindow;
    class VersionsPage;

    class VersionInfo :
        public Gtk::TreeView,
        private paludis::PrivateImplementationPattern<VersionInfo>
    {
        protected:
            void populate_in_paludis_thread();
            void populate_in_gui_thread();

        public:
            VersionInfo(QueryWindow * const m, VersionsPage * const p);
            ~VersionInfo();

            void populate();
    };
}

#endif
