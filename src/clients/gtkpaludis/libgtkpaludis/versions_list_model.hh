/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef GTKPALUDIS_GUARD_LIBGTKPALUDIS_VERSIONS_LIST_MODEL_HH
#define GTKPALUDIS_GUARD_LIBGTKPALUDIS_VERSIONS_LIST_MODEL_HH 1

#include <gtkmm/treestore.h>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/name.hh>
#include <paludis/package_database_entry.hh>

namespace gtkpaludis
{
    class QueryWindow;
    class VersionsPage;

    class VersionsListModel :
        private paludis::PrivateImplementationPattern<VersionsListModel>,
        public Gtk::TreeStore
    {
        protected:
            class PopulateData;

            void populate_in_paludis_thread();
            void populate_in_gui_thread(paludis::tr1::shared_ptr<const PopulateData> names);

        public:
            VersionsListModel(QueryWindow * const m, VersionsPage * const p);
            ~VersionsListModel();

            class Columns :
                public Gtk::TreeModelColumnRecord
            {
                public:
                    Columns();
                    ~Columns();

                    Gtk::TreeModelColumn<Glib::ustring> col_version_string;
                    Gtk::TreeModelColumn<Glib::ustring> col_repo_name;
                    Gtk::TreeModelColumn<Glib::ustring> col_slot;
                    Gtk::TreeModelColumn<Glib::ustring> col_masks_markup;
                    Gtk::TreeModelColumn<paludis::tr1::shared_ptr<const paludis::PackageDatabaseEntry> > col_pde;
                    Gtk::TreeModelColumn<bool> col_prefer_default;
            };

            Columns & columns();

            void populate();
    };
}

#endif
