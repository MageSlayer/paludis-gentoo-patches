/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef GTKPALUDIS_GUARD_LIBGTKPALUDIS_QUERY_WINDOW_HH
#define GTKPALUDIS_GUARD_LIBGTKPALUDIS_QUERY_WINDOW_HH 1

#include <libgtkpaludis/threaded_window.hh>
#include <paludis/util/private_implementation_pattern.hh>

namespace paludis
{
    class QualifiedPackageName;
}

namespace gtkpaludis
{
    class MainWindow;

    class QueryWindow :
        public ThreadedWindow,
        private paludis::PrivateImplementationPattern<QueryWindow>
    {
        private:
            paludis::PrivateImplementationPattern<QueryWindow>::ImpPtr & _imp;

        protected:
            virtual bool handle_delete_event(GdkEventAny *);
            void handle_ok_button_clicked();
            virtual void do_set_sensitive(const bool);

            virtual void push_status_message(const std::string &);
            virtual void pop_status_message();

        public:
            QueryWindow(MainWindow * const, const paludis::QualifiedPackageName &);
            ~QueryWindow();

            const paludis::QualifiedPackageName get_package_name() const;
    };
}

#endif
