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

#include "tasks_page.hh"
#include <gtkmm/box.h>
#include <gtkmm/image.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <gtkmm/textview.h>
#include <gtkmm/table.h>
#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>
#include <gtkmm/stock.h>
#include <cellrendererbutton/cellrendererbutton.hh>
#include <limits>

#include "config.h"

using namespace paludis;
using namespace gtkpaludis;

namespace
{
    struct Columns :
        Gtk::TreeModel::ColumnRecord
    {
        Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > col_icon;
        Gtk::TreeModelColumn<Glib::ustring> col_text;
        Gtk::TreeModelColumn<Glib::ustring> col_button;

        Columns()
        {
            add(col_icon);
            add(col_text);
            add(col_button);
        }
    };

    struct TasksList :
        Gtk::TreeView
    {
        Columns columns;
        Glib::RefPtr<Gtk::ListStore> model;

        TasksList() :
            model(Gtk::ListStore::create(columns))
        {
            set_model(model);
            set_headers_visible(false);
#ifdef HAVE_TREE_VIEW_GRID_LINES
            set_grid_lines(Gtk::TREE_VIEW_GRID_LINES_VERTICAL);
#endif

            {
                get_column(append_column("Icon", columns.col_icon) - 1)->get_first_cell_renderer()->
                    set_fixed_size(40, 40);
                get_column(append_column("Text", columns.col_text) - 1)->set_expand(true);

                CellRendererButton * const renderer = new CellRendererButton(*this);
                Gtk::TreeViewColumn * const column = new Gtk::TreeViewColumn("Button",
                        *Gtk::manage(renderer));
                column->add_attribute(renderer->property_text(), columns.col_button);
                renderer->property_width_chars() = 10;
                renderer->set_column(column);
                append_column(*column);
            }

            Gtk::TreeModel::Row row = *(model->append());
            row[columns.col_icon] = render_icon(Gtk::Stock::MISSING_IMAGE, Gtk::ICON_SIZE_LARGE_TOOLBAR);
            row[columns.col_text] = "Sync all repositories";
            row[columns.col_button] = "Sync";

            row = *(model->append());
            row[columns.col_icon] = render_icon(Gtk::Stock::MISSING_IMAGE, Gtk::ICON_SIZE_LARGE_TOOLBAR);
            row[columns.col_text] = "Show security updates";
            row[columns.col_button] = "Preview";

            row = *(model->append());
            row[columns.col_icon] = render_icon(Gtk::Stock::MISSING_IMAGE, Gtk::ICON_SIZE_LARGE_TOOLBAR);
            row[columns.col_text] = "Show world updates";
            row[columns.col_button] = "Preview";
        }
    };
}

namespace paludis
{
    template<>
    struct Implementation<TasksPage> :
        InternalCounted<Implementation<TasksPage> >
    {
        TasksList tasks_list;
    };
}

TasksPage::TasksPage() :
    PrivateImplementationPattern<TasksPage>(new Implementation<TasksPage>)
{
    set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_ALWAYS);
    add(_imp->tasks_list);
}

TasksPage::~TasksPage()
{
}

