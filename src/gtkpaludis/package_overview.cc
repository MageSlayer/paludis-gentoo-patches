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

#include "package_overview.hh"
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
            Gtk::TreeModelColumn<Glib::ustring> col_left;
            Gtk::TreeModelColumn<Glib::ustring> col_right;

            Columns()
            {
                add(col_left);
                add(col_right);
            }
    };
}

namespace paludis
{
    template<>
    struct Implementation<PackageOverview> :
        InternalCounted<Implementation<PackageOverview> >
    {
        PackageOverview * const overview;
        Columns columns;
        Glib::RefPtr<Gtk::TreeStore> model;

        Implementation(PackageOverview * const o) :
            overview(o),
            model(Gtk::TreeStore::create(columns))
        {
        }

        void set_model(const Glib::RefPtr<Gtk::TreeStore> & m)
        {
            model = m;
            overview->set_model(model);
            overview->expand_all();
        }
    };
}

PackageOverview::PackageOverview() :
    PrivateImplementationPattern<PackageOverview>(new Implementation<PackageOverview>(this))
{
    set_headers_visible(false);
    set_model(_imp->model);
    append_column("Left", _imp->columns.col_left);
    append_column("Right", _imp->columns.col_right);
}

PackageOverview::~PackageOverview()
{
}

namespace
{
    class Populate :
        public PaludisThread::Launchable
    {
        private:
            Implementation<PackageOverview> * const _imp;
            QualifiedPackageName _pkg;

        public:
            Populate(Implementation<PackageOverview> * const imp, const QualifiedPackageName & pkg) :
                _imp(imp),
                _pkg(pkg)
            {
            }

            virtual void operator() ();
    };

    void
    Populate::operator() ()
    {
        Glib::RefPtr<Gtk::TreeStore> model(Gtk::TreeStore::create(_imp->columns));
        std::map<RepositoryName, Gtk::TreeModel::iterator> repository_rows;

        Gtk::TreeModel::Row top_row = *model->append();
        top_row[_imp->columns.col_left] = stringify(_pkg);
        top_row[_imp->columns.col_right] = "";

        dispatch(sigc::bind<1>(sigc::mem_fun(MainWindow::get_instance(),
                        &MainWindow::push_status), "Querying package versions..."));

        PackageDatabaseEntryCollection::ConstPointer results(DefaultEnvironment::get_instance()->package_database()->query(
                    PackageDepAtom::Pointer(new PackageDepAtom(stringify(_pkg))), is_either));

        for (PackageDatabaseEntryCollection::Iterator i(results->begin()), i_end(results->end()) ;
                i != i_end ; ++i)
        {
            std::map<RepositoryName, Gtk::TreeModel::iterator>::iterator r(repository_rows.find(i->repository));
            if (repository_rows.end() == r)
            {
                r = repository_rows.insert(std::make_pair(i->repository, model->append(top_row.children()))).first;
                (*r->second)[_imp->columns.col_left] = stringify(i->repository);
            }

            Glib::ustring value((*r->second)[_imp->columns.col_right]);
            if (! value.empty())
                value.append(" ");
            value.append(stringify(i->version));
            (*r->second)[_imp->columns.col_right] = value;
        }

        dispatch(sigc::mem_fun(MainWindow::get_instance(), &MainWindow::pop_status));

        dispatch(sigc::bind<1>(sigc::mem_fun(_imp, &Implementation<PackageOverview>::set_model), model));
    }
}

void
PackageOverview::populate(const QualifiedPackageName & name)
{
    if (name == QualifiedPackageName("no-category/no-package"))
        _imp->set_model(Gtk::TreeStore::create(_imp->columns));
    else
        PaludisThread::get_instance()->launch(Populate::Pointer(new Populate(_imp.raw_pointer(), name)));
}

