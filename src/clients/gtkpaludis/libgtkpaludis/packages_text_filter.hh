/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef GTKPALUDIS_GUARD_LIBGTKPALUDIS_PACKAGES_TEXT_FILTER_HH
#define GTKPALUDIS_GUARD_LIBGTKPALUDIS_PACKAGES_TEXT_FILTER_HH 1

#include <gtkmm/table.h>
#include <paludis/util/private_implementation_pattern.hh>

namespace gtkpaludis
{
    class MainWindow;
    class PackagesPage;

    class PackagesTextFilter :
        private paludis::PrivateImplementationPattern<PackagesTextFilter>,
        public Gtk::Table
    {
        private:
            void handle_filter_entry_changed();

        public:
            PackagesTextFilter(MainWindow * const m, PackagesPage * const p);
            ~PackagesTextFilter();

            void populate();
    };
}

#endif
