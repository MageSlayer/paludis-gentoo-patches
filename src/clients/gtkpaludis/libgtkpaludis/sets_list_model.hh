/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef GTKPALUDIS_GUARD_LIBGTKPALUDIS_SETS_LIST_MODEL_HH
#define GTKPALUDIS_GUARD_LIBGTKPALUDIS_SETS_LIST_MODEL_HH 1

#include <gtkmm/liststore.h>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/name.hh>
#include <tr1/memory>

namespace gtkpaludis
{
    class MainWindow;
    class PackagesPage;

    class SetsListModel :
        private paludis::PrivateImplementationPattern<SetsListModel>,
        public Gtk::ListStore
    {
        protected:
            void populate_in_paludis_thread();
            void populate_in_gui_thread(const std::tr1::shared_ptr<const paludis::SetNameSet> & names);

        public:
            SetsListModel(MainWindow * const m, PackagesPage * const p);
            ~SetsListModel();

            void populate();

            class Columns :
                public Gtk::TreeModelColumnRecord
            {
                public:
                    Columns();
                    ~Columns();

                    Gtk::TreeModelColumn<Glib::ustring> col_set_name;
            };

            Columns & columns();
    };
}


#endif
