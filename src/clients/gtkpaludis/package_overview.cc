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
            overview->columns_autosize();
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
    std::string mask_reasons_to_letters(const MaskReasons & masks)
    {
        std::string reasons;
        for (MaskReason m(MaskReason(0)) ; m < last_mr ;
                m = MaskReason(static_cast<int>(m) + 1))
        {
            if (! masks.test(m))
                continue;

            switch (m)
            {
                case mr_keyword:
                    reasons.append("K");
                    break;
                case mr_user_mask:
                    reasons.append("U");
                    break;
                case mr_profile_mask:
                    reasons.append("P");
                    break;
                case mr_repository_mask:
                    reasons.append("R");
                    break;
                case mr_eapi:
                    reasons.append("E");
                    break;
                case mr_license:
                    reasons.append("L");
                    break;
                case mr_by_association:
                    reasons.append("A");
                    break;
                case last_mr:
                    break;
            }
        }

        return reasons;
    }

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
        StatusBarMessage m1(this, "Querying package...");

        Glib::RefPtr<Gtk::TreeStore> model(Gtk::TreeStore::create(_imp->columns));
        std::map<RepositoryName, Gtk::TreeModel::iterator, RepositoryNameComparator> repository_rows;

        Gtk::TreeModel::Row top_row = *model->append();
        top_row[_imp->columns.col_left] = stringify(_pkg);
        top_row[_imp->columns.col_right] = "";

        Gtk::TreeModel::Row description_row = *model->append(top_row.children());
        description_row[_imp->columns.col_left] = "Description";

        Gtk::TreeModel::Row homepage_row = *model->append(top_row.children());
        homepage_row[_imp->columns.col_left] = "Homepage";

        Gtk::TreeModel::Row versions_row = *model->append(top_row.children());
        versions_row[_imp->columns.col_left] = "Versions";
        versions_row[_imp->columns.col_right] = "";

        PackageDepAtom::Pointer atom(new PackageDepAtom(stringify(_pkg)));
        PackageDatabaseEntryCollection::ConstPointer
            entries(DefaultEnvironment::get_instance()->package_database()->query(atom, is_any)),
            preferred_entries(DefaultEnvironment::get_instance()->package_database()->query(atom, is_installed_only));
        if (preferred_entries->empty())
            preferred_entries = entries;

        {
            StatusBarMessage m2(this, "Querying package versions...");

            for (PackageDatabaseEntryCollection::Iterator i(entries->begin()), i_end(entries->end()) ;
                    i != i_end ; ++i)
            {
                std::map<RepositoryName, Gtk::TreeModel::iterator, RepositoryNameComparator>::iterator
                    r(repository_rows.find(i->repository));
                if (repository_rows.end() == r)
                {
                    r = repository_rows.insert(std::make_pair(i->repository, model->append(versions_row.children()))).first;
                    (*r->second)[_imp->columns.col_left] = stringify(i->repository);
                }

                Glib::ustring value((*r->second)[_imp->columns.col_right]);
                if (! value.empty())
                    value.append(" ");
                value.append(stringify(i->version));

                std::string reasons(mask_reasons_to_letters(DefaultEnvironment::get_instance()->mask_reasons(*i)));
                if (! reasons.empty())
                    value.append("(" + reasons + ")");

                (*r->second)[_imp->columns.col_right] = value;
            }
        }

        if (! (preferred_entries->empty()))
        {
            StatusBarMessage m2(this, "Querying package metadata...");
            VersionMetadata::ConstPointer metadata(DefaultEnvironment::get_instance()->package_database()->fetch_repository(
                        preferred_entries->last()->repository)->version_metadata(
                        preferred_entries->last()->name, preferred_entries->last()->version));
            homepage_row[_imp->columns.col_right] = metadata->homepage;
            description_row[_imp->columns.col_right] = metadata->description;
        }

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

