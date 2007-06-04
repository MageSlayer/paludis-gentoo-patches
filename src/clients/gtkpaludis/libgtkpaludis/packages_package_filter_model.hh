/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef GTKPALUDIS_GUARD_LIBGTKPALUDIS_PACKAGES_PACKAGE_FILTER_MODEL_HH
#define GTKPALUDIS_GUARD_LIBGTKPALUDIS_PACKAGES_PACKAGE_FILTER_MODEL_HH 1

#include <gtkmm/treestore.h>
#include <paludis/util/private_implementation_pattern.hh>
#include <libgtkpaludis/packages_package_filter_option.hh>

namespace gtkpaludis
{
    class MainWindow;

    class PackagesPackageFilterModel :
        private paludis::PrivateImplementationPattern<PackagesPackageFilterModel>,
        public Gtk::TreeStore
    {
        public:
            PackagesPackageFilterModel(MainWindow * const m);
            ~PackagesPackageFilterModel();

            void populate();

            class Columns :
                public Gtk::TreeModelColumnRecord
            {
                public:
                    Columns();
                    ~Columns();

                    Gtk::TreeModelColumn<Glib::ustring> col_text;
                    Gtk::TreeModelColumn<PackagesPackageFilterOption> col_filter;
            };

            Columns & columns();
    };
}

#endif
