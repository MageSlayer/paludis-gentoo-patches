/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef GTKPALUDIS_GUARD_LIBGTKPALUDIS_PACKAGES_LIST_MODEL_HH
#define GTKPALUDIS_GUARD_LIBGTKPALUDIS_PACKAGES_LIST_MODEL_HH 1

#include <gtkmm/treestore.h>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/tr1_functional.hh>
#include <paludis/util/tr1_memory.hh>
#include <paludis/name.hh>
#include <libgtkpaludis/packages_package_filter_option.hh>

namespace gtkpaludis
{
    class MainWindow;
    class PackagesPage;

    class PackagesListModel :
        private paludis::PrivateImplementationPattern<PackagesListModel>,
        public Gtk::TreeStore
    {
        protected:
            class PopulateData;
            class PopulateDataIterator;

            void _populate_in_gui_thread_recursive(
                    Gtk::TreeNodeChildren &,
                    PopulateDataIterator,
                    PopulateDataIterator);

            void populate_in_paludis_thread();
            void populate_in_gui_thread(paludis::tr1::shared_ptr<const PopulateData> names);

        public:
            PackagesListModel(MainWindow * const m, PackagesPage * const p);
            ~PackagesListModel();

            class Columns :
                public Gtk::TreeModelColumnRecord
            {
                public:
                    Columns();
                    ~Columns();

                    Gtk::TreeModelColumn<Glib::ustring> col_package;
                    Gtk::TreeModelColumn<Glib::ustring> col_status_markup;
                    Gtk::TreeModelColumn<Glib::ustring> col_description;
                    Gtk::TreeModelColumn<paludis::tr1::shared_ptr<const paludis::QualifiedPackageName> > col_qpn;
                    Gtk::TreeModelColumn<PackagesPackageFilterOption> col_best_package_filter_option;
            };

            Columns & columns();

            void populate();
    };
}

#endif
