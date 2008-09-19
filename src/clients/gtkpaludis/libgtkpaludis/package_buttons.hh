/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef GTKPALUDIS_GUARD_LIBGTKPALUDIS_PACKAGE_BUTTONS_HH
#define GTKPALUDIS_GUARD_LIBGTKPALUDIS_PACKAGE_BUTTONS_HH 1

#include <gtkmm/buttonbox.h>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/name.hh>
#include <tr1/memory>

namespace gtkpaludis
{
    class MainWindow;
    class PackagesPage;

    class PackageButtons :
        public Gtk::HButtonBox,
        private paludis::PrivateImplementationPattern<PackageButtons>
    {
        private:
            void populate_in_paludis_thread(const std::tr1::shared_ptr<const paludis::QualifiedPackageName> &);
            void populate_in_gui_thread(const bool, const bool, const bool);

        protected:
            void handle_query_button_clicked();

        public:
            PackageButtons(MainWindow * const m, PackagesPage * const p);
            ~PackageButtons();

            void populate();
    };
}

#endif
