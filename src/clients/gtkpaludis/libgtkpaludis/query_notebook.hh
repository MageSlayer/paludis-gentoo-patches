/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef GTKPALUDIS_GUARD_LIBGTKPALUDIS_QUERY_NOTEBOOK_HH
#define GTKPALUDIS_GUARD_LIBGTKPALUDIS_QUERY_NOTEBOOK_HH 1

#include <gtkmm/notebook.h>
#include <paludis/util/private_implementation_pattern.hh>

namespace gtkpaludis
{
    class QueryWindow;

    class QueryNotebook :
        public Gtk::Notebook,
        private paludis::PrivateImplementationPattern<QueryNotebook>
    {
        protected:
            void handle_switch_page(GtkNotebookPage *, guint);

        public:
            QueryNotebook(QueryWindow * const);
            ~QueryNotebook();

            void populate();
    };
}

#endif
