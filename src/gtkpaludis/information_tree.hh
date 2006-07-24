/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#ifndef PALUDIS_GUARD_SRC_GTKPALUDIS_INFORMATION_TREE_HH
#define PALUDIS_GUARD_SRC_GTKPALUDIS_INFORMATION_TREE_HH 1

#include <gtkmm/treeview.h>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/name.hh>
#include <paludis/version_spec.hh>

namespace paludis
{
    class InformationTreeColumns :
        public Gtk::TreeModel::ColumnRecord
    {
        public:
            InformationTreeColumns();
            ~InformationTreeColumns();

            Gtk::TreeModelColumn<Glib::ustring> col_key;
            Gtk::TreeModelColumn<Glib::ustring> col_value;
    };

    class InformationTree :
        public Gtk::TreeView,
        private PrivateImplementationPattern<InformationTree>
    {
        public:
            InformationTree();
            ~InformationTree();

            void show_top();
            void show_repository(const RepositoryName &);
            void show_category(const RepositoryName &, const CategoryNamePart &);
            void show_package(const RepositoryName &, const QualifiedPackageName &);
            void show_version(const RepositoryName &, const QualifiedPackageName &,
                    const VersionSpec &);
    };
}

#endif
