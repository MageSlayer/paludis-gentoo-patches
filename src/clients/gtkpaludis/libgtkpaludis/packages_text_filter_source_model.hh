/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef GTKPALUDIS_GUARD_LIBGTKPALUDIS_PACKAGES_TEXT_FILTER_SOURCE_MODEL_HH
#define GTKPALUDIS_GUARD_LIBGTKPALUDIS_PACKAGES_TEXT_FILTER_SOURCE_MODEL_HH 1

#include <gtkmm/treestore.h>
#include <paludis/util/private_implementation_pattern.hh>
#include <libgtkpaludis/packages_text_filter_source_option.hh>

namespace gtkpaludis
{
    class MainWindow;

    class PackagesTextFilterSourceModel :
        private paludis::PrivateImplementationPattern<PackagesTextFilterSourceModel>,
        public Gtk::TreeStore
    {
        public:
            PackagesTextFilterSourceModel(MainWindow * const m);
            ~PackagesTextFilterSourceModel();

            void populate();

            class Columns :
                public Gtk::TreeModelColumnRecord
            {
                public:
                    Columns();
                    ~Columns();

                    Gtk::TreeModelColumn<Glib::ustring> col_text;
                    Gtk::TreeModelColumn<PackagesTextFilterSourceOption> col_filter;
            };

            Columns & columns();
    };
}

#endif
