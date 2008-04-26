/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef GTKPALUDIS_GUARD_LIBGTKPALUDIS_REPOSITORY_INFO_MODEL_HH
#define GTKPALUDIS_GUARD_LIBGTKPALUDIS_REPOSITORY_INFO_MODEL_HH 1

#include <gtkmm/treestore.h>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/name.hh>
#include <tr1/memory>

namespace paludis
{
    class RepositoryInfo;
}

namespace gtkpaludis
{
    class MainWindow;

    class RepositoryInfoModel :
        private paludis::PrivateImplementationPattern<RepositoryInfoModel>,
        public Gtk::TreeStore
    {
        private:
#if 0
            void set_repository_in_paludis_thread(const paludis::RepositoryName &);
            void set_repository_in_gui_thread(std::tr1::shared_ptr<const paludis::RepositoryInfo>);
#endif

        public:
            RepositoryInfoModel(MainWindow * const);
            ~RepositoryInfoModel();

            void set_repository(const paludis::RepositoryName &);

            class Columns :
                public Gtk::TreeModelColumnRecord
            {
                public:
                    Columns();
                    ~Columns();

                    Gtk::TreeModelColumn<Glib::ustring> col_key;
                    Gtk::TreeModelColumn<Glib::ustring> col_value;
            };

            Columns & columns();
    };
}

#endif
