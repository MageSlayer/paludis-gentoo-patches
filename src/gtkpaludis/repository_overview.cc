/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
 *
 * This file is part of the Paludis package manager. Paludis is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License version 2, as published by the Free Software Foundation.
 *
 * Paludis is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "repository_overview.hh"
#include "paludis_thread.hh"
#include "main_window.hh"

#include <paludis/environment/default/default_environment.hh>
#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>

using namespace gtkpaludis;
using namespace paludis;

namespace
{
    class Columns :
        public Gtk::TreeModel::ColumnRecord
    {
        public:
            Gtk::TreeModelColumn<Glib::ustring> col_key;
            Gtk::TreeModelColumn<Glib::ustring> col_value;

            Columns()
            {
                add(col_key);
                add(col_value);
            }
    };
}

namespace paludis
{
    template<>
    struct Implementation<RepositoryOverview> :
        InternalCounted<Implementation<RepositoryOverview> >
    {
        RepositoryOverview * const info;
        Columns columns;
        Glib::RefPtr<Gtk::TreeStore> model;

        Implementation(RepositoryOverview * const o) :
            info(o),
            model(Gtk::TreeStore::create(columns))
        {
        }

        void set_model(const Glib::RefPtr<Gtk::TreeStore> & m)
        {
            model = m;
            info->set_model(model);
            info->expand_all();
            info->columns_autosize();
        }
    };
}

RepositoryOverview::RepositoryOverview() :
    PrivateImplementationPattern<RepositoryOverview>(new Implementation<RepositoryOverview>(this))
{
    set_headers_visible(false);
    set_model(_imp->model);
    append_column("Key", _imp->columns.col_key);
    append_column("Value", _imp->columns.col_value);
}

RepositoryOverview::~RepositoryOverview()
{
}

namespace
{
    class Populate :
        public PaludisThread::Launchable
    {
        private:
            Implementation<RepositoryOverview> * const _imp;
            RepositoryName _repo;

        public:
            Populate(Implementation<RepositoryOverview> * const imp, const RepositoryName & repo) :
                _imp(imp),
                _repo(repo)
            {
            }

            virtual void operator() ();
    };

    void
    Populate::operator() ()
    {
        Glib::RefPtr<Gtk::TreeStore> model(Gtk::TreeStore::create(_imp->columns));

        Gtk::TreeModel::Row top_row = *model->append();
        top_row[_imp->columns.col_key] = stringify(_repo);
        top_row[_imp->columns.col_value] = "";

        StatusBarMessage m1(this, "Querying repository information...");

        RepositoryInfo::ConstPointer info(DefaultEnvironment::get_instance()->package_database()->fetch_repository(
                    _repo)->info(true));
        for (RepositoryInfo::SectionIterator s(info->begin_sections()), s_end(info->end_sections()) ;
                s != s_end ; ++s)
        {
            Gtk::TreeModel::Row section_row = *model->append(top_row.children());
            section_row[_imp->columns.col_key] = (*s)->heading();
            section_row[_imp->columns.col_value] = "";

            for (RepositoryInfoSection::KeyValueIterator k((*s)->begin_kvs()), k_end((*s)->end_kvs()) ; k != k_end ; ++k)
            {
                Gtk::TreeModel::Row kv_row = *model->append(section_row->children());
                kv_row[_imp->columns.col_key] = k->first;
                kv_row[_imp->columns.col_value] = k->second;
            }
        }

        dispatch(sigc::bind<1>(sigc::mem_fun(_imp, &Implementation<RepositoryOverview>::set_model), model));
    }
}

void
RepositoryOverview::populate(const RepositoryName & name)
{
    if (name.data() == "no-repository")
        _imp->set_model(Gtk::TreeStore::create(_imp->columns));
    else
        PaludisThread::get_instance()->launch(Populate::Pointer(new Populate(_imp.raw_pointer(), name)));
}


