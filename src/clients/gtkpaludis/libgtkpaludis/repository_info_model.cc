/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "repository_info_model.hh"
#include "main_window.hh"
#include <paludis/repository.hh>
#include <paludis/environment.hh>
#include <paludis/package_database.hh>
#include <paludis/util/iterator.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>

using namespace gtkpaludis;
using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<RepositoryInfoModel>
    {
        MainWindow * const main_window;
        RepositoryInfoModel::Columns columns;

        Implementation(MainWindow * const m) :
            main_window(m)
        {
        }
    };
}

RepositoryInfoModel::RepositoryInfoModel(MainWindow * const m) :
    PrivateImplementationPattern<RepositoryInfoModel>(new Implementation<RepositoryInfoModel>(m)),
    Gtk::TreeStore(_imp->columns)
{
}

RepositoryInfoModel::~RepositoryInfoModel()
{
}

void
RepositoryInfoModel::set_repository(const RepositoryName & name)
{
    _imp->main_window->paludis_thread_action(
            sigc::bind(sigc::mem_fun(this, &RepositoryInfoModel::set_repository_in_paludis_thread), name),
            "Populating repository information model");
}

void
RepositoryInfoModel::set_repository_in_paludis_thread(const RepositoryName & name)
{
    paludis::tr1::shared_ptr<const RepositoryInfo> info(
            _imp->main_window->environment()->package_database()->fetch_repository(name)->info(true));

    _imp->main_window->gui_thread_action(
            sigc::bind(sigc::mem_fun(this, &RepositoryInfoModel::set_repository_in_gui_thread), info));
}

void
RepositoryInfoModel::set_repository_in_gui_thread(paludis::tr1::shared_ptr<const RepositoryInfo> info)
{
    clear();
    for (IndirectIterator<RepositoryInfo::SectionIterator>
            s(indirect_iterator(info->begin_sections())), s_end(indirect_iterator(info->end_sections())) ;
            s != s_end ; ++s)
    {
        iterator i(append());
        (*i)[_imp->columns.col_key] = s->heading();

        for (RepositoryInfoSection::KeyValueIterator k(s->begin_kvs()), k_end(s->end_kvs()) ;
                k != k_end ; ++k)
        {
            iterator j(append(i->children()));
            (*j)[_imp->columns.col_key] = k->first;
            (*j)[_imp->columns.col_value] = k->second;
        }
    }
}

RepositoryInfoModel::Columns::Columns()
{
    add(col_key);
    add(col_value);
}

RepositoryInfoModel::Columns::~Columns()
{
}

RepositoryInfoModel::Columns &
RepositoryInfoModel::columns()
{
    return _imp->columns;
}


