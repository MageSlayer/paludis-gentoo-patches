/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "packages_page.hh"
#include "packages_filter.hh"
#include "categories_list.hh"
#include "sets_list.hh"
#include "packages_list.hh"
#include "package_buttons.hh"

#include <paludis/util/private_implementation_pattern-impl.hh>
#include <gtkmm/scrolledwindow.h>

using namespace gtkpaludis;
using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<PackagesPage>
    {
        MainWindow * const main_window;

        PackagesFilter packages_filter;

        Gtk::ScrolledWindow categories_list_scroll;
        CategoriesList categories_list;

        Gtk::ScrolledWindow sets_list_scroll;
        SetsList sets_list;

        Gtk::ScrolledWindow packages_list_scroll;
        PackagesList packages_list;
        PackageButtons package_buttons;

        std::tr1::shared_ptr<const Generator> repository_filter;
        std::tr1::shared_ptr<const CategoryNamePart> category;
        std::tr1::shared_ptr<const SetName> set;
        std::tr1::shared_ptr<const QualifiedPackageName> qpn;
        PackagesPackageFilterOption package_filter;
        PackagesTextFilterSourceOption text_filter;
        std::string text_filter_text;

        Implementation(MainWindow * const m, PackagesPage * const p) :
            main_window(m),
            packages_filter(m, p),
            categories_list(m, p),
            sets_list(m, p),
            packages_list(m, p),
            package_buttons(m, p),
            repository_filter(new generator::All()),
            package_filter(ppfo_all_packages),
            text_filter(ptfso_name)
        {
        }
    };
}

PackagesPage::PackagesPage(MainWindow * const m) :
    Gtk::Table(4, 2),
    MainNotebookPage(),
    PrivateImplementationPattern<PackagesPage>(new Implementation<PackagesPage>(m, this))
{
    attach(_imp->packages_filter, 0, 2, 0, 1, Gtk::FILL, Gtk::FILL, 4, 4);

    _imp->categories_list_scroll.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_ALWAYS);
    _imp->categories_list_scroll.add(_imp->categories_list);
    attach(_imp->categories_list_scroll, 0, 1, 1, 2, Gtk::FILL, Gtk::EXPAND | Gtk::FILL, 4, 4);

    _imp->sets_list_scroll.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_ALWAYS);
    _imp->sets_list_scroll.add(_imp->sets_list);
    attach(_imp->sets_list_scroll, 0, 1, 2, 3, Gtk::FILL, Gtk::EXPAND | Gtk::FILL, 4, 4);

    _imp->packages_list_scroll.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    _imp->packages_list_scroll.add(_imp->packages_list);

    attach(_imp->packages_list_scroll, 1, 2, 1, 3, Gtk::EXPAND | Gtk::FILL, Gtk::EXPAND | Gtk::FILL, 4, 4);
    attach(_imp->package_buttons, 0, 2, 3, 4, Gtk::FILL, Gtk::FILL, 4, 4);
}

PackagesPage::~PackagesPage()
{
}

void
PackagesPage::populate()
{
    _imp->categories_list.populate();
    _imp->sets_list.populate();
    _imp->packages_filter.populate();
    _imp->packages_list.populate_real();
    _imp->package_buttons.populate();
}

void
PackagesPage::set_category(std::tr1::shared_ptr<const CategoryNamePart> c)
{
    _imp->category = c;
    _imp->set.reset();
    _imp->packages_list.populate_real();
    _imp->package_buttons.populate();
}

void
PackagesPage::set_set(std::tr1::shared_ptr<const SetName> c)
{
    _imp->set = c;
    _imp->category.reset();
    _imp->packages_list.populate_real();
    _imp->package_buttons.populate();
}

std::tr1::shared_ptr<const CategoryNamePart>
PackagesPage::get_category() const
{
    return _imp->category;
}

std::tr1::shared_ptr<const SetName>
PackagesPage::get_set() const
{
    return _imp->set;
}

void
PackagesPage::set_qpn(std::tr1::shared_ptr<const QualifiedPackageName> q)
{
    _imp->qpn = q;
    _imp->package_buttons.populate();
}

std::tr1::shared_ptr<const QualifiedPackageName>
PackagesPage::get_qpn() const
{
    return _imp->qpn;
}

void
PackagesPage::set_repository_filter(std::tr1::shared_ptr<const Generator> q)
{
    _imp->repository_filter = q;
    _imp->categories_list.populate();
    _imp->sets_list.populate();
    _imp->packages_list.populate_real();
    _imp->package_buttons.populate();
}

std::tr1::shared_ptr<const Generator>
PackagesPage::get_repository_filter() const
{
    return _imp->repository_filter;
}

void
PackagesPage::set_package_filter(const PackagesPackageFilterOption q)
{
    _imp->package_filter = q;
    _imp->packages_list.populate_filter();
}

PackagesPackageFilterOption
PackagesPage::get_package_filter() const
{
    return _imp->package_filter;
}

void
PackagesPage::set_text_filter(const PackagesTextFilterSourceOption q)
{
    _imp->text_filter = q;
    _imp->packages_list.populate_filter();
}

PackagesTextFilterSourceOption
PackagesPage::get_text_filter() const
{
    return _imp->text_filter;
}

void
PackagesPage::set_text_filter_text(const std::string & q)
{
    _imp->text_filter_text = q;
    _imp->packages_list.populate_filter();
}

std::string
PackagesPage::get_text_filter_text() const
{
    return _imp->text_filter_text;
}

