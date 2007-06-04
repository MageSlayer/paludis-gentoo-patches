/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef GTKPALUDIS_GUARD_LIBGTKPALUDIS_PACKAGES_TEXT_FILTER_SOURCE_HH
#define GTKPALUDIS_GUARD_LIBGTKPALUDIS_PACKAGES_TEXT_FILTER_SOURCE_HH 1

#include <gtkmm/combobox.h>
#include <paludis/util/private_implementation_pattern.hh>

namespace gtkpaludis
{
    class MainWindow;
    class PackagesPage;

    class PackagesTextFilterSource :
        private paludis::PrivateImplementationPattern<PackagesTextFilterSource>,
        public Gtk::ComboBox
    {
        protected:
            void handle_signal_changed();

        public:
            PackagesTextFilterSource(MainWindow * const, PackagesPage * const);
            ~PackagesTextFilterSource();

            void populate();
    };
}

#endif
