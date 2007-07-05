/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef GTKPALUDIS_GUARD_LIBGTKPALUDIS_REPOSITORIES_LIST_MODEL_HH
#define GTKPALUDIS_GUARD_LIBGTKPALUDIS_REPOSITORIES_LIST_MODEL_HH 1

#include <gtkmm/liststore.h>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/tr1_memory.hh>
#include <paludis/name.hh>

namespace gtkpaludis
{
    class MainWindow;

    class RepositoriesListModel :
        private paludis::PrivateImplementationPattern<RepositoriesListModel>,
        public Gtk::ListStore
    {
        protected:
            void populate_in_paludis_thread();
            void populate_in_gui_thread(paludis::tr1::shared_ptr<const paludis::RepositoryNameSequence> names);

        public:
            RepositoriesListModel(MainWindow * const m);
            ~RepositoriesListModel();

            void populate();

            class Columns :
                public Gtk::TreeModelColumnRecord
            {
                public:
                    Columns();
                    ~Columns();

                    Gtk::TreeModelColumn<Glib::ustring> col_repo_name;
            };

            Columns & columns();
    };
}

#endif
