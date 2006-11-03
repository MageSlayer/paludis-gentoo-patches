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

#include "queue_list.hh"
#include "queue_page.hh"
#include "main_window.hh"
#include "install.hh"
#include "paludis_thread.hh"
#include <gtkmm/liststore.h>
#include <paludis/util/stringify.hh>
#include <list>

using namespace paludis;
using namespace gtkpaludis;

namespace
{
    class Columns :
        public Gtk::TreeModel::ColumnRecord
    {
        public:
            Gtk::TreeModelColumn<Glib::ustring> col_package;
            Gtk::TreeModelColumn<Glib::ustring> col_status;
            Gtk::TreeModelColumn<Glib::ustring> col_use;
            Gtk::TreeModelColumn<Glib::ustring> col_tags;
            Gtk::TreeModelColumn<Glib::ustring> col_why;

            Columns()
            {
                add(col_package);
                add(col_status);
                add(col_use);
                add(col_tags);
                add(col_why);
            }
    };
}

namespace paludis
{
    template<>
    struct Implementation<QueueList> :
        InternalCounted<Implementation<QueueList> >
    {
        QueuePage * const page;

        Columns columns;
        Glib::RefPtr<Gtk::ListStore> model;
        OurInstallTask install_task;

        Implementation(QueuePage * const p) :
            page(p),
            model(Gtk::ListStore::create(columns))
        {
        }
    };
}

QueueList::QueueList(QueuePage * const page) :
    PrivateImplementationPattern<QueueList>(new Implementation<QueueList>(page))
{
    set_model(_imp->model);
}

QueueList::~QueueList()
{
}

void
QueueList::clear()
{
    _imp->install_task.clear();
    invalidate();
}

void
QueueList::add_target(const std::string & s)
{
    try
    {
        _imp->install_task.add_target(s);
        invalidate();
    }
    catch (const HadBothPackageAndSetTargets &)
    {
        MainWindow::get_instance()->show_error_dialog("Cannot add target '" + s + "'",
                "Cannot add target '" + s + "' because the queue already includes package targets. Package "
                "and set targets cannot both be specified in the same transaction.");
    }
    catch (const MultipleSetTargetsSpecified &)
    {
        MainWindow::get_instance()->show_error_dialog("Cannot add target '" + s + "'",
                "Cannot add target '" + s + "' because the queue already includes a set target. Multiple "
                "set targets cannot be specified in the same transaction.");
    }
}

void
QueueList::invalidate()
{
    _imp->page->set_queue_list_calculated(false);
    remove_all_columns();
    append_column("Target", _imp->columns.col_package);

    Glib::RefPtr<Gtk::ListStore> new_model(Gtk::ListStore::create(_imp->columns));

    for (InstallTask::TargetsIterator i(_imp->install_task.begin_targets()),
            i_end(_imp->install_task.end_targets()) ; i != i_end ; ++i)
    {
        Gtk::TreeModel::Row row = *(new_model->append());
        row[_imp->columns.col_package] = stringify(*i);
    }

    _imp->model.swap(new_model);
    set_model(_imp->model);
}

namespace gtkpaludis
{
    class QueueList::Populate :
        public PaludisThread::Launchable,
        public OurInstallTask::Callbacks
    {
        private:
            QueueList * const _q;
            Glib::RefPtr<Gtk::ListStore> _model;

        public:
            Populate(QueueList * const q, Glib::RefPtr<Gtk::ListStore> model) :
                _q(q),
                _model(model)
            {
            }

            virtual void operator() ();
            virtual void display_entry(const paludis::DepListEntry & e);
    };
}

void
QueueList::Populate::operator() ()
{
    StatusBarMessage m1(this, "Building dependency list...");

    _q->_imp->install_task.set_pretend(true);
    _q->_imp->install_task.execute(this);

    dispatch(sigc::bind<1>(sigc::mem_fun(_q, &QueueList::set_model_show_dep_columns), _model));
}

void
QueueList::Populate::display_entry(const paludis::DepListEntry & e)
{
    if (e.already_installed)
        return;

    Gtk::TreeModel::Row row = *(_model->append());
    row[_q->_imp->columns.col_package] = stringify(e.package);
}

void
QueueList::calculate()
{
    PaludisThread::get_instance()->launch(Populate::Pointer(new Populate(this,
                    Gtk::ListStore::create(_imp->columns))));
}

void
QueueList::set_model_show_dep_columns(Glib::RefPtr<Gtk::ListStore> new_model)
{
    remove_all_columns();
    append_column("Package", _imp->columns.col_package);
    append_column("", _imp->columns.col_status);
    append_column("Use", _imp->columns.col_use);
    append_column("Tags", _imp->columns.col_tags);
    append_column("Why", _imp->columns.col_why);

    _imp->model.swap(new_model);
    set_model(_imp->model);

    _imp->page->set_queue_list_calculated(true);
}

