/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef GTKPALUDIS_GUARD_GTKPALUDIS_LIBGTKPALUDIS_REPOSITORIES_PAGE_HH
#define GTKPALUDIS_GUARD_GTKPALUDIS_LIBGTKPALUDIS_REPOSITORIES_PAGE_HH 1

#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/name.hh>
#include <libgtkpaludis/main_notebook_page.hh>
#include <gtkmm/table.h>

namespace gtkpaludis
{
    class MainWindow;

    class RepositoriesPage :
        public Gtk::Table,
        public MainNotebookPage,
        private paludis::PrivateImplementationPattern<RepositoriesPage>
    {
        public:
            RepositoriesPage(MainWindow * const m);
            ~RepositoriesPage();

            virtual void populate();

            void set_repository(const paludis::RepositoryName &);
    };
}

#endif
