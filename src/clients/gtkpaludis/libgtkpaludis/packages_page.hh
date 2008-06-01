/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef GTKPALUDIS_GUARD_LIBGTKPALUDIS_PACKAGES_PAGE_HH
#define GTKPALUDIS_GUARD_LIBGTKPALUDIS_PACKAGES_PAGE_HH 1

#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/name.hh>
#include <paludis/generator.hh>
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

            void set_category(std::tr1::shared_ptr<const paludis::CategoryNamePart>);
            std::tr1::shared_ptr<const paludis::CategoryNamePart> get_category() const;

            void set_set(std::tr1::shared_ptr<const paludis::SetName>);
            std::tr1::shared_ptr<const paludis::SetName> get_set() const;

            void set_repository_filter(std::tr1::shared_ptr<const paludis::Generator>);
            std::tr1::shared_ptr<const paludis::Generator> get_repository_filter() const;

            void set_package_filter(const PackagesPackageFilterOption);
            PackagesPackageFilterOption get_package_filter() const;

            void set_text_filter(const PackagesTextFilterSourceOption);
            PackagesTextFilterSourceOption get_text_filter() const;

            void set_text_filter_text(const std::string &);
            std::string get_text_filter_text() const;

            void set_qpn(std::tr1::shared_ptr<const paludis::QualifiedPackageName>);
            std::tr1::shared_ptr<const paludis::QualifiedPackageName> get_qpn() const;

    };
}

#endif
