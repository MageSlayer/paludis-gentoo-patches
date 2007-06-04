/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "repository_info.hh"
#include "repository_info_model.hh"
#include <paludis/util/private_implementation_pattern-impl.hh>

using namespace gtkpaludis;

namespace paludis
{
    template<>
    struct Implementation<gtkpaludis::RepositoryInfo>
    {
        MainWindow * const main_window;

        Glib::RefPtr<RepositoryInfoModel> model;

        Implementation(MainWindow * const m) :
            main_window(m),
            model(new RepositoryInfoModel(m))
        {
        }
    };
}

RepositoryInfo::RepositoryInfo(MainWindow * const m) :
    Gtk::TreeView(),
    paludis::PrivateImplementationPattern<RepositoryInfo>(new paludis::Implementation<RepositoryInfo>(m))
{
    set_model(_imp->model);

    append_column("Key", _imp->model->columns().col_key);
    append_column("Value", _imp->model->columns().col_value);
}

RepositoryInfo::~RepositoryInfo()
{
}

void
RepositoryInfo::set_repository(const paludis::RepositoryName & name)
{
    _imp->model->set_repository(name);
}

