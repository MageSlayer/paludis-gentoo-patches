/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef GTKPALUDIS_GUARD_LIBGTKPALUDIS_REPOSITORY_INFO_HH
#define GTKPALUDIS_GUARD_LIBGTKPALUDIS_REPOSITORY_INFO_HH 1

#include <gtkmm/treeview.h>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/name.hh>

namespace gtkpaludis
{
    class MainWindow;

    class RepositoryInfo :
        public Gtk::TreeView,
        private paludis::PrivateImplementationPattern<RepositoryInfo>
    {
        public:
            RepositoryInfo(MainWindow * const);
            ~RepositoryInfo();

            void set_repository(const paludis::RepositoryName &);
    };
}

#endif
