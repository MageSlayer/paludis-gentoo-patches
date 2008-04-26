/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef GTKPALUDIS_GUARD_LIBGTKPALUDIS_PACKAGES_LIST_FILTERED_MODEL_HH
#define GTKPALUDIS_GUARD_LIBGTKPALUDIS_PACKAGES_LIST_FILTERED_MODEL_HH 1

#include <gtkmm/treemodelfilter.h>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/name.hh>
#include <tr1/memory>

namespace gtkpaludis
{
    class MainWindow;
    class PackagesPage;

    class PackagesListModel;

    class PackagesListFilteredModel :
        private paludis::PrivateImplementationPattern<PackagesListFilteredModel>,
        public Gtk::TreeModelFilter
    {
        private:
            bool handle_visible_func(const TreeModel::const_iterator &) const;

        protected:
            class PopulateData;

            void populate_in_paludis_thread();
            void populate_in_gui_thread(std::tr1::shared_ptr<const PopulateData> names);

        public:
            PackagesListFilteredModel(MainWindow * const m, PackagesPage * const p,
                    Glib::RefPtr<PackagesListModel>);
            ~PackagesListFilteredModel();

            void populate();
    };
}

#endif
