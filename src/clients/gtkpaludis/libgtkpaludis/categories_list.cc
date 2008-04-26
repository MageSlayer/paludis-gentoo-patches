/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "categories_list.hh"
#include "categories_list_model.hh"
#include "packages_page.hh"
#include <paludis/util/private_implementation_pattern-impl.hh>

using namespace gtkpaludis;
using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<CategoriesList>
    {
        Glib::RefPtr<CategoriesListModel> model;
        MainWindow * const main_window;
        PackagesPage * const packages_page;

        Implementation(MainWindow * const m, PackagesPage * const p) :
            model(new CategoriesListModel(m, p)),
            main_window(m),
            packages_page(p)
        {
        }
    };
}

CategoriesList::CategoriesList(MainWindow * const m, PackagesPage * const p) :
    Gtk::TreeView(),
    PrivateImplementationPattern<CategoriesList>(new Implementation<CategoriesList>(m, p))
{
    set_model(_imp->model);

    append_column("Category", _imp->model->columns().col_cat_name);

    signal_cursor_changed().connect(sigc::mem_fun(this, &CategoriesList::handle_signal_cursor_changed));
}

CategoriesList::~CategoriesList()
{
}

void
CategoriesList::populate()
{
    _imp->model->populate();
}

void
CategoriesList::handle_signal_cursor_changed()
{
    if (get_selection()->get_selected())
        _imp->packages_page->set_category(std::tr1::shared_ptr<CategoryNamePart>(new CategoryNamePart(
                        static_cast<Glib::ustring>((*get_selection()->get_selected())[_imp->model->columns().col_cat_name]).raw())));
    else
        _imp->packages_page->set_category(
                std::tr1::shared_ptr<CategoryNamePart>());
}


