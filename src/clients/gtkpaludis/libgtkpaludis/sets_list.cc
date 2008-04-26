/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "sets_list.hh"
#include "sets_list_model.hh"
#include "packages_page.hh"
#include <paludis/util/private_implementation_pattern-impl.hh>

using namespace gtkpaludis;
using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<SetsList>
    {
        Glib::RefPtr<SetsListModel> model;
        MainWindow * const main_window;
        PackagesPage * const packages_page;

        Implementation(MainWindow * const m, PackagesPage * const p) :
            model(new SetsListModel(m, p)),
            main_window(m),
            packages_page(p)
        {
        }
    };
}

SetsList::SetsList(MainWindow * const m, PackagesPage * const p) :
    Gtk::TreeView(),
    PrivateImplementationPattern<SetsList>(new Implementation<SetsList>(m, p))
{
    set_model(_imp->model);

    append_column("Set", _imp->model->columns().col_set_name);

    signal_cursor_changed().connect(sigc::mem_fun(this, &SetsList::handle_signal_cursor_changed));
}

SetsList::~SetsList()
{
}

void
SetsList::populate()
{
    _imp->model->populate();
}

void
SetsList::handle_signal_cursor_changed()
{
    if (get_selection()->get_selected())
        _imp->packages_page->set_set(std::tr1::shared_ptr<SetName>(new SetName(
                        static_cast<Glib::ustring>((*get_selection()->get_selected())[_imp->model->columns().col_set_name]).raw())));
    else
        _imp->packages_page->set_set(
                std::tr1::shared_ptr<SetName>());
}

