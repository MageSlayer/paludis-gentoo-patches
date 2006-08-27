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

#include "information_tree.hh"
#include <gtkmm/treestore.h>
#include <paludis/default_environment.hh>

using namespace paludis;

InformationTreeColumns::InformationTreeColumns()
{
    add(col_key);
    add(col_value);
}

InformationTreeColumns::~InformationTreeColumns()
{
}

namespace paludis
{
    template<>
    struct Implementation<InformationTree> :
        InternalCounted<Implementation<InformationTree> >
    {
        InformationTreeColumns columns;
        Glib::RefPtr<Gtk::TreeStore> model;
    };
}

InformationTree::InformationTree() :
    PrivateImplementationPattern<InformationTree>(new Implementation<InformationTree>)
{
    _imp->model = Gtk::TreeStore::create(_imp->columns);
    set_model(_imp->model);

    append_column("Key", _imp->columns.col_key);
    append_column("Value", _imp->columns.col_value);
}

InformationTree::~InformationTree()
{
}

void
InformationTree::show_top()
{
    set_model(Glib::RefPtr<Gtk::TreeStore>(0));
    _imp->model = Gtk::TreeStore::create(_imp->columns);

    set_model(_imp->model);
}

void
InformationTree::show_repository(const RepositoryName & r)
{
    Repository::ConstPointer repo(DefaultEnvironment::get_instance()->package_database()->fetch_repository(r));

    set_model(Glib::RefPtr<Gtk::TreeStore>(0));
    _imp->model = Gtk::TreeStore::create(_imp->columns);

    Gtk::TreeModel::Row name_row(*(_imp->model->append()));
    name_row[_imp->columns.col_key] = "name";
    name_row[_imp->columns.col_value] = stringify(repo->name());

    RepositoryInfo::ConstPointer info(repo->info(false));
    for (RepositoryInfo::SectionIterator section(info->begin_sections()),
            section_end(info->end_sections()) ; section != section_end ; ++section)
    {
        Gtk::TreeModel::Row section_row(*(_imp->model->append()));
        section_row[_imp->columns.col_key] = (*section)->heading();

        for (RepositoryInfoSection::KeyValueIterator kv((*section)->begin_kvs()),
                kv_end((*section)->end_kvs()) ; kv != kv_end ; ++kv)
        {
            Gtk::TreeModel::Row kv_row(*(_imp->model->append(section_row.children())));
            kv_row[_imp->columns.col_key] = kv->first;
            kv_row[_imp->columns.col_value] = kv->second;
        }
    }

    set_model(_imp->model);
}

void
InformationTree::show_category(const RepositoryName & r, const CategoryNamePart & c)
{
    Repository::ConstPointer repo(DefaultEnvironment::get_instance()->package_database()->fetch_repository(r));

    set_model(Glib::RefPtr<Gtk::TreeStore>(0));
    _imp->model = Gtk::TreeStore::create(_imp->columns);

    Gtk::TreeModel::Row name_row(*(_imp->model->append()));
    name_row[_imp->columns.col_key] = "name";
    name_row[_imp->columns.col_value] = stringify(c);

    set_model(_imp->model);
}

void
InformationTree::show_package(const RepositoryName & r, const QualifiedPackageName & q)
{
    Repository::ConstPointer repo(DefaultEnvironment::get_instance()->package_database()->fetch_repository(r));

    set_model(Glib::RefPtr<Gtk::TreeStore>(0));
    _imp->model = Gtk::TreeStore::create(_imp->columns);

    Gtk::TreeModel::Row name_row(*(_imp->model->append()));
    name_row[_imp->columns.col_key] = "name";
    name_row[_imp->columns.col_value] = stringify(q);

    set_model(_imp->model);
}


void
InformationTree::show_version(const RepositoryName & r, const QualifiedPackageName & q,
        const VersionSpec & v)
{
    Repository::ConstPointer repo(DefaultEnvironment::get_instance()->package_database()->fetch_repository(r));

    set_model(Glib::RefPtr<Gtk::TreeStore>(0));
    _imp->model = Gtk::TreeStore::create(_imp->columns);

    Gtk::TreeModel::Row name_row(*(_imp->model->append()));
    name_row[_imp->columns.col_key] = "name";
    name_row[_imp->columns.col_value] = stringify(q);

    Gtk::TreeModel::Row version_row(*(_imp->model->append()));
    version_row[_imp->columns.col_key] = "version";
    version_row[_imp->columns.col_value] = stringify(v);

    VersionMetadata::ConstPointer metadata(repo->version_metadata(q, v));

    Gtk::TreeModel::Row description_row(*(_imp->model->append()));
    description_row[_imp->columns.col_key] = "description";
    description_row[_imp->columns.col_value] = metadata->description;

    Gtk::TreeModel::Row homepage_row(*(_imp->model->append()));
    description_row[_imp->columns.col_key] = "homepage";
    description_row[_imp->columns.col_value] = metadata->homepage;

    set_model(_imp->model);
}

