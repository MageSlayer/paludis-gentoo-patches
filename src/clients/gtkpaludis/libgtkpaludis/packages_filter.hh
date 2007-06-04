/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef GTKPALUDIS_GUARD_LIBGTKPALUDIS_PACKAGES_FILTER_HH
#define GTKPALUDIS_GUARD_LIBGTKPALUDIS_PACKAGES_FILTER_HH 1

#include <gtkmm/box.h>
#include <paludis/util/private_implementation_pattern.hh>

namespace gtkpaludis
{
    class MainWindow;
    class PackagesPage;

    class PackagesFilter :
        public Gtk::VBox,
        private paludis::PrivateImplementationPattern<PackagesFilter>
    {
        public:
            PackagesFilter(MainWindow * const, PackagesPage * const);
            ~PackagesFilter();

            void populate();
    };
}

#endif
