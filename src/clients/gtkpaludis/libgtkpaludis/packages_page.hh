/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef GTKPALUDIS_GUARD_LIBGTKPALUDIS_PACKAGES_PAGE_HH
#define GTKPALUDIS_GUARD_LIBGTKPALUDIS_PACKAGES_PAGE_HH 1

#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/name.hh>
#include <paludis/query.hh>
#include <paludis/package_database_entry.hh>
#include <libgtkpaludis/main_notebook_page.hh>
#include <libgtkpaludis/packages_package_filter_option.hh>
#include <libgtkpaludis/packages_text_filter_source_option.hh>
#include <gtkmm/table.h>

namespace gtkpaludis
{
    class MainWindow;

    class PackagesPage :
        public Gtk::Table,
        public MainNotebookPage,
        private paludis::PrivateImplementationPattern<PackagesPage>
    {
        public:
            PackagesPage(MainWindow * const m);
            ~PackagesPage();

            virtual void populate();

            void set_category(paludis::tr1::shared_ptr<const paludis::CategoryNamePart>);
            paludis::tr1::shared_ptr<const paludis::CategoryNamePart> get_category() const;

            void set_repository_filter(paludis::tr1::shared_ptr<const paludis::Query>);
            paludis::tr1::shared_ptr<const paludis::Query> get_repository_filter() const;

            void set_package_filter(const PackagesPackageFilterOption);
            PackagesPackageFilterOption get_package_filter() const;

            void set_text_filter(const PackagesTextFilterSourceOption);
            PackagesTextFilterSourceOption get_text_filter() const;

            void set_text_filter_text(const std::string &);
            std::string get_text_filter_text() const;

            void set_qpn(paludis::tr1::shared_ptr<const paludis::QualifiedPackageName>);
            paludis::tr1::shared_ptr<const paludis::QualifiedPackageName> get_qpn() const;

    };
}

#endif
