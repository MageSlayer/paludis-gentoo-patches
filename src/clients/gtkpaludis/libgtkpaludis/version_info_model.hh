/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef GTKPALUDIS_GUARD_LIBGTKPALUDIS_VERSION_INFO_MODEL_HH
#define GTKPALUDIS_GUARD_LIBGTKPALUDIS_VERSION_INFO_MODEL_HH 1

#include <gtkmm/treestore.h>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/name.hh>
#include <tr1/memory>

namespace gtkpaludis
{
    class QueryWindow;
    class VersionsPage;

    class VersionInfoModel :
        private paludis::PrivateImplementationPattern<VersionInfoModel>,
        public Gtk::TreeStore
    {
        protected:
            class MetadataPopulator;
            class KeyMetadataPopulator;
            class MaskMetadataPopulator;
            class MaskPopulator;
            class PopulateData;

            void populate_in_paludis_thread(const std::tr1::shared_ptr<const paludis::PackageID> &);
            void populate_in_gui_thread(const std::tr1::shared_ptr<const PopulateData> & names);

        public:
            VersionInfoModel(QueryWindow * const m, VersionsPage * const p);
            ~VersionInfoModel();

            class Columns :
                public Gtk::TreeModelColumnRecord
            {
                public:
                    Columns();
                    ~Columns();

                    Gtk::TreeModelColumn<Glib::ustring> col_key;
                    Gtk::TreeModelColumn<Glib::ustring> col_value_markup;
            };

            Columns & columns();

            void populate();
    };
}

#endif
