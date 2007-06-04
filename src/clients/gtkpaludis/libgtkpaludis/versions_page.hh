/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef GTKPALUDIS_GUARD_LIBGTKPALUDIS_VERSIONS_PAGE_HH
#define GTKPALUDIS_GUARD_LIBGTKPALUDIS_VERSIONS_PAGE_HH 1

#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/package_database_entry.hh>
#include <libgtkpaludis/query_notebook_page.hh>
#include <gtkmm/table.h>

namespace gtkpaludis
{
    class QueryWindow;

    class VersionsPage :
        public Gtk::Table,
        public QueryNotebookPage,
        private paludis::PrivateImplementationPattern<VersionsPage>
    {
        public:
            VersionsPage(QueryWindow * const m);
            ~VersionsPage();

            virtual void populate();

            void set_pde(paludis::tr1::shared_ptr<const paludis::PackageDatabaseEntry>);
            paludis::tr1::shared_ptr<const paludis::PackageDatabaseEntry> get_pde() const;
    };
}

#endif
