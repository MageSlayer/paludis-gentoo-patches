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
#include "repositories_page.hh"
#include "main_window.hh"
#include "queue_page.hh"

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/image.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <gtkmm/textview.h>
#include <gtkmm/table.h>
#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/stock.h>
#include <limits>

#include "config.h"

#ifdef USE_BROKEN_CELL_RENDERER_BUTTON
#  include <cellrendererbutton/cellrendererbutton.hh>
#endif

using namespace paludis;
using namespace gtkpaludis;

namespace
{
    struct Columns :
        Gtk::TreeModel::ColumnRecord
    {
        Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > col_icon;
        Gtk::TreeModelColumn<Glib::ustring> col_text;
#ifdef USE_BROKEN_CELL_RENDERER_BUTTON
        Gtk::TreeModelColumn<Glib::ustring> col_button;
#endif
        Gtk::TreeModelColumn<sigc::slot<void> > col_action;

        Columns()
        {
            add(col_icon);
            add(col_text);
#ifdef USE_BROKEN_CELL_RENDERER_BUTTON
            add(col_button);
#endif
            add(col_action);
        }
    };

    struct TasksList :
        Gtk::TreeView
    {
        Columns columns;
        Glib::RefPtr<Gtk::ListStore> model;

        TasksList(TasksPage * const page) :
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

#ifdef USE_BROKEN_CELL_RENDERER_BUTTON
                CellRendererButton * const renderer = new CellRendererButton(*this);
                Gtk::TreeViewColumn * const column = new Gtk::TreeViewColumn("Button",
                        *Gtk::manage(renderer));
                column->add_attribute(renderer->property_text(), columns.col_button);
                renderer->property_width_chars() = 10;
                renderer->set_column(column);
                append_column(*column);
#endif
            }

            Gtk::TreeModel::Row row = *(model->append());
            row[columns.col_icon] = render_icon(Gtk::Stock::MISSING_IMAGE, Gtk::ICON_SIZE_LARGE_TOOLBAR);
            row[columns.col_text] = "Sync all repositories";
#ifdef USE_BROKEN_CELL_RENDERER_BUTTON
            row[columns.col_button] = "Sync";
#endif
            row[columns.col_action] = sigc::mem_fun(page, &TasksPage::sync_action);

            row = *(model->append());
            row[columns.col_icon] = render_icon(Gtk::Stock::MISSING_IMAGE, Gtk::ICON_SIZE_LARGE_TOOLBAR);
            row[columns.col_text] = "Show security updates";
#ifdef USE_BROKEN_CELL_RENDERER_BUTTON
            row[columns.col_button] = "Preview";
#endif
            row[columns.col_action] = sigc::mem_fun(page, &TasksPage::security_action);

            row = *(model->append());
            row[columns.col_icon] = render_icon(Gtk::Stock::MISSING_IMAGE, Gtk::ICON_SIZE_LARGE_TOOLBAR);
            row[columns.col_text] = "Show world updates";
#ifdef USE_BROKEN_CELL_RENDERER_BUTTON
            row[columns.col_button] = "Preview";
#endif
            row[columns.col_action] = sigc::mem_fun(page, &TasksPage::world_action);
        }
    };
}

namespace paludis
{
    template<>
    struct Implementation<TasksPage> :
        InternalCounted<Implementation<TasksPage> >
    {
        Gtk::ScrolledWindow tasks_list_window;
        TasksList tasks_list;
        Gtk::HButtonBox buttons_box;
        Gtk::Button go_button;

        Implementation(TasksPage * const page) :
            tasks_list(page),
            go_button("Go")
        {
        }
    };
}

TasksPage::TasksPage() :
    PrivateImplementationPattern<TasksPage>(new Implementation<TasksPage>(this))
{
    _imp->tasks_list_window.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_ALWAYS);
    _imp->tasks_list_window.add(_imp->tasks_list);
    pack_start(_imp->tasks_list_window, Gtk::PACK_EXPAND_WIDGET);
    pack_start(_imp->buttons_box, Gtk::PACK_SHRINK);

    _imp->buttons_box.set_border_width(5);
    _imp->buttons_box.set_spacing(5);
    _imp->buttons_box.set_layout(Gtk::BUTTONBOX_END);
    _imp->buttons_box.add(_imp->go_button);

    _imp->go_button.set_sensitive(false);
    _imp->go_button.signal_clicked().connect(sigc::mem_fun(this,
                &TasksPage::_go_button_clicked));
    _imp->tasks_list.get_selection()->signal_changed().connect(sigc::mem_fun(this,
                &TasksPage::_task_list_selection_changed));
}

TasksPage::~TasksPage()
{
}

void
TasksPage::_task_list_selection_changed()
{
    _imp->go_button.set_sensitive(_imp->tasks_list.get_selection()->get_selected());
}

void
TasksPage::sync_action()
{
    MainWindow::get_instance()->repositories_page()->sync_all();
}

void
TasksPage::security_action()
{
    MainWindow::get_instance()->queue_page()->add_target("security");
    MainWindow::get_instance()->show_queue_page();
    MainWindow::get_instance()->queue_page()->recalculate();
}

void
TasksPage::world_action()
{
    MainWindow::get_instance()->queue_page()->add_target("world");
    MainWindow::get_instance()->show_queue_page();
    MainWindow::get_instance()->queue_page()->recalculate();
}

void
TasksPage::_go_button_clicked()
{
    Gtk::TreeModel::iterator i(_imp->tasks_list.get_selection()->get_selected());
    if (i)
    {
        sigc::slot<void> slot = (*i)[_imp->tasks_list.columns.col_action];
        slot();
    }
}

