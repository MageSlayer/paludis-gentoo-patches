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

#include "categories_list.hh"
#include "paludis_thread.hh"
#include "main_window.hh"
#include <paludis/environment/default/default_environment.hh>

#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>
#include <glibmm/dispatcher.h>
#include <set>

using namespace paludis;
using namespace gtkpaludis;

namespace
{
    class Columns :
        public Gtk::TreeModel::ColumnRecord
    {
        public:
            Gtk::TreeModelColumn<Glib::ustring> col_category;

            Columns()
            {
                add(col_category);
            }
    };
}

namespace paludis
{
    template<>
    struct Implementation<CategoriesList> :
        InternalCounted<Implementation<CategoriesList> >
    {
        Columns columns;
        Glib::RefPtr<Gtk::ListStore> model;

        Implementation() :
            model(Gtk::ListStore::create(columns))
        {
        }

        void add_categories(const std::set<CategoryNamePart> & c)
        {
            for (std::set<CategoryNamePart>::const_iterator n(c.begin()), n_end(c.end()) ; n != n_end ; ++n)
            {
                Gtk::TreeModel::Row row = *(model->append());
                row[columns.col_category] = stringify(*n);
            }
        }
    };
}

namespace
{
    class Populate :
        public PaludisThread::Launchable
    {
        private:
            Implementation<CategoriesList> * const _imp;

        public:
            Populate(Implementation<CategoriesList> * const list) :
                _imp(list)
            {
            }

            virtual void operator() ();
    };

    void
    Populate::operator() ()
    {
        std::set<CategoryNamePart> names;

        StatusBarMessage m1(this, "Loading category names...");

        for (PackageDatabase::RepositoryIterator
                r(DefaultEnvironment::get_instance()->package_database()->begin_repositories()),
                r_end(DefaultEnvironment::get_instance()->package_database()->end_repositories()) ; r != r_end ; ++r)
        {
            StatusBarMessage m2(this, "Loading category names from '" + stringify((*r)->name()) + "'...");

            CategoryNamePartCollection::ConstPointer cats((*r)->category_names());
            std::copy(cats->begin(), cats->end(), std::inserter(names, names.end()));
        }

        dispatch(sigc::bind<1>(sigc::mem_fun(_imp, &Implementation<CategoriesList>::add_categories), names));
    }
}

CategoriesList::CategoriesList() :
    PrivateImplementationPattern<CategoriesList>(new Implementation<CategoriesList>)
{
    set_model(_imp->model);
    append_column("Category", _imp->columns.col_category);
}

CategoriesList::~CategoriesList()
{
}

void
CategoriesList::populate()
{
    _imp->model->clear();
    PaludisThread::get_instance()->launch(Populate::Pointer(new Populate(_imp.raw_pointer())));
}

CategoryNamePart
CategoriesList::current_category()
{
    Gtk::TreeModel::iterator i(get_selection()->get_selected());
    if (i)
        return CategoryNamePart(stringify((*i)[_imp->columns.col_category]));
    else
        return CategoryNamePart("no-category");
}

