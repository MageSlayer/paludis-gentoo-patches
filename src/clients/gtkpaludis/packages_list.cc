/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include "packages_list.hh"
#include "paludis_thread.hh"
#include "main_window.hh"
#include <paludis/environment/default/default_environment.hh>
#include <paludis/util/sr.hh>
#include <paludis/util/compare.hh>

#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>
#include <glibmm/dispatcher.h>
#include <map>

using namespace gtkpaludis;
using namespace paludis;

namespace gtkpaludis
{
#include "packages_list-sr.hh"
#include "packages_list-sr.cc"
}

namespace
{
    class Columns :
        public Gtk::TreeModel::ColumnRecord
    {
        public:
            Gtk::TreeModelColumn<Glib::ustring> col_package_display;
            Gtk::TreeModelColumn<bool> col_is_installed;
            Gtk::TreeModelColumn<Glib::ustring> col_package_real;
            Gtk::TreeModelColumn<Glib::ustring> col_description;

            Columns()
            {
                add(col_package_display);
                add(col_is_installed);
                add(col_package_real);
                add(col_description);
            }
    };
}

namespace paludis
{
    template<>
    struct Implementation<PackagesList>
    {
        Columns columns;
        Glib::RefPtr<Gtk::ListStore> model;
        PackagesList * const list;

        sigc::signal<void> populated;

        Implementation(PackagesList * const l) :
            model(Gtk::ListStore::create(columns)),
            list(l)
        {
        }

        void add_packages(const std::map<QualifiedPackageName, PackagesListEntry> & s)
        {
            for (std::map<QualifiedPackageName, PackagesListEntry>::const_iterator n(s.begin()), n_end(s.end()) ;
                    n != n_end ; ++n)
            {
                Gtk::TreeModel::Row row = *(model->append());
                row[columns.col_package_display] = stringify(n->first.package);
                row[columns.col_package_real] = stringify(n->first);
                row[columns.col_is_installed] = n->second.is_installed;
                row[columns.col_description] = n->second.description;
            }

            list->columns_autosize();
        }
    };
}

namespace
{
}

PackagesList::PackagesList() :
    PrivateImplementationPattern<PackagesList>(new Implementation<PackagesList>(this))
{
    set_model(_imp->model);
    append_column("Package", _imp->columns.col_package_display);
    append_column("I", _imp->columns.col_is_installed);
    append_column("Description", _imp->columns.col_description);
}

PackagesList::~PackagesList()
{
}

namespace
{
    class Populate :
        public PaludisThread::Launchable
    {
        private:
            Implementation<PackagesList> * const _list;
            CategoryNamePart _cat;

        public:
            Populate(Implementation<PackagesList> * const list, const CategoryNamePart & cat) :
                _list(list),
                _cat(cat)
            {
            }

            virtual void operator() ();
    };

    void
    Populate::operator() ()
    {
        std::map<QualifiedPackageName, PackagesListEntry> names;

        {
            StatusBarMessage m1(this, "Loading package names...");

            for (PackageDatabase::RepositoryIterator
                    r(DefaultEnvironment::get_instance()->package_database()->begin_repositories()),
                    r_end(DefaultEnvironment::get_instance()->package_database()->end_repositories()) ; r != r_end ; ++r)
            {
                StatusBarMessage m2(this, "Loading package names from '" + stringify((*r)->name()) + "'...");

                std::tr1::shared_ptr<const QualifiedPackageNameCollection> pkgs((*r)->package_names(_cat));
                for (QualifiedPackageNameCollection::Iterator p(pkgs->begin()), p_end(pkgs->end()) ;
                        p != p_end ; ++p)
                    names.insert(std::make_pair(*p, PackagesListEntry::create().description("").is_installed(false)));
            }
        }

        {
            StatusBarMessage m1(this, "Loading package descriptions...");

            for (std::map<QualifiedPackageName, PackagesListEntry>::iterator i(names.begin()), i_end(names.end()) ;
                    i != i_end ; ++i)
            {
                bool is_installed(true);

                std::tr1::shared_ptr<const PackageDatabaseEntryCollection> results(DefaultEnvironment::get_instance()->package_database()->query(
                            PackageDepAtom(i->first), is_installed_only, qo_order_by_version));
                if (results->empty())
                {
                    is_installed = false;
                    results = DefaultEnvironment::get_instance()->package_database()->query(
                            PackageDepAtom(i->first), is_any, qo_order_by_version);
                }
                if (results->empty())
                    continue;

                i->second.description = DefaultEnvironment::get_instance()->package_database()->fetch_repository(
                        results->last()->repository)->version_metadata(results->last()->name,
                        results->last()->version)->description;
                i->second.is_installed = is_installed;
            }
        }

        dispatch(sigc::bind<1>(sigc::mem_fun(_list, &Implementation<PackagesList>::add_packages), names));
        dispatch(sigc::mem_fun(_list->populated, &sigc::signal<void>::operator()));
    }
}

void
PackagesList::clear_packages()
{
    _imp->model->clear();
}

void
PackagesList::populate(const CategoryNamePart & cat)
{
    clear_packages();
    if (cat.data() != "no-category")
        PaludisThread::get_instance()->launch(std::tr1::shared_ptr<Populate>(new Populate(_imp.operator-> (), cat)));
}

QualifiedPackageName
PackagesList::current_package()
{
    Gtk::TreeModel::iterator i(get_selection()->get_selected());
    if (i)
        return QualifiedPackageName(stringify((*i)[_imp->columns.col_package_real]));
    else
        return QualifiedPackageName("no-category/no-package");
}

sigc::signal<void> &
PackagesList::populated()
{
    return _imp->populated;
}

int
PackagesList::number_of_packages()
{
    return _imp->model->children().size();
}

