/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef GTKPALUDIS_GUARD_LIBGTKPALUDIS_PACKAGES_PACKAGE_FILTER_HH
#define GTKPALUDIS_GUARD_LIBGTKPALUDIS_PACKAGES_PACKAGE_FILTER_HH 1

#include <gtkmm/combobox.h>
#include <paludis/util/private_implementation_pattern.hh>

namespace gtkpaludis
{
    class MainWindow;
    class PackagesPage;

    class PackagesPackageFilter :
        private paludis::PrivateImplementationPattern<PackagesPackageFilter>,
        public Gtk::ComboBox
    {
        protected:
            void handle_signal_changed();

        public:
            PackagesPackageFilter(MainWindow * const, PackagesPage * const);
            ~PackagesPackageFilter();

            void populate();
    };
}

#endif
