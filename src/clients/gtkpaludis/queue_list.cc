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

#include "queue_list.hh"
#include "queue_page.hh"
#include "main_window.hh"
#include "install.hh"
#include "paludis_thread.hh"

#include <gtkmm/liststore.h>
#include <gtkmm/stock.h>
#include <gdkmm/pixbuf.h>

#include <paludis/util/stringify.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/log.hh>
#include <paludis/tasks/exceptions.hh>
#include <paludis/environment/default/default_environment.hh>

#include <list>
#include <set>

using namespace paludis;
using namespace gtkpaludis;

namespace
{
    class Columns :
        public Gtk::TreeModel::ColumnRecord
    {
        public:
            Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > col_icon;
            Gtk::TreeModelColumn<Glib::ustring> col_left;
            Gtk::TreeModelColumn<Glib::ustring> col_right;

            Columns()
            {
                add(col_icon);
                add(col_left);
                add(col_right);
            }
    };

    template <typename T_>
    std::string
    pango_colour(const std::string & c, const T_ & s)
    {
        return "<span foreground=\"" + c + "\">" + stringify(s) + "</span>";
    }

    std::string::size_type
    use_expand_delim_pos(const UseFlagName & u, const std::tr1::shared_ptr<const UseFlagNameCollection> c)
    {
        for (UseFlagNameCollection::Iterator i(c->begin()), i_end(c->end()) ; i != i_end ; ++i)
            if (0 == u.data().compare(0, i->data().length(), i->data(), 0, i->data().length()))
                return i->data().length();
        return std::string::npos;
    }

    void
    add_use_rows(Glib::RefPtr<Gtk::TreeStore> & model,
            Gtk::TreeModel::Row & row,
            Gtk::TreeModelColumn<Glib::ustring> & col_left,
            Gtk::TreeModelColumn<Glib::ustring> & col_right,
            const Environment * const env, const PackageDatabaseEntry & p,
            std::tr1::shared_ptr<const VersionMetadata> metadata, const PackageDatabaseEntry * const other_p)
    {
        static const std::string cl_flag_on("#00cc00");
        static const std::string cl_flag_off("#cc0000");

        if (metadata->ebuild_interface)
        {
            std::string use_expand_string;
            UseFlagName expand_name("OFTEN_NOT_BEEN_ON_BOATS"), expand_value("MONKEY");

            const RepositoryUseInterface * const use_interface(
                    env->package_database()->
                    fetch_repository(p.repository)->use_interface);
            std::set<UseFlagName> iuse;
            WhitespaceTokeniser::get_instance()->tokenise(
                    metadata->ebuild_interface->iuse,
                    create_inserter<UseFlagName>(std::inserter(iuse, iuse.end())));

            /* display normal use flags first */
            std::string use_string;
            for (std::set<UseFlagName>::const_iterator i(iuse.begin()), i_end(iuse.end()) ;
                    i != i_end ; ++i)
            {
                if (std::string::npos != use_expand_delim_pos(*i, use_interface->use_expand_prefixes()))
                    continue;

                if (! use_string.empty())
                    use_string.append(" ");

                if (env->query_use(*i, &p))
                {
                    if (use_interface && use_interface->query_use_force(*i, &p))
                        use_string.append(pango_colour(cl_flag_on, "(" + stringify(*i) + ")"));
                    else
                        use_string.append(pango_colour(cl_flag_on, *i));
                }
                else
                {
                    if (use_interface && use_interface->query_use_mask(*i, &p))
                        use_string.append(pango_colour(cl_flag_off, "(-" + stringify(*i) + ")"));
                    else
                        use_string.append(pango_colour(cl_flag_off, "-" + stringify(*i)));
                }

                if (other_p)
                    if (env->query_use(*i, &p) != env->query_use(*i, other_p))
                        use_string.append("*");
            }

            if (! use_string.empty())
            {
                Gtk::TreeModel::Row use_row = *(model->append(row->children()));
                use_row[col_left] = "USE:";
                use_row[col_right] = use_string;
            }

            /* now display expand flags */
            UseFlagName old_expand_name("OFTEN_NOT_BEEN_ON_BOATS");
            for (std::set<UseFlagName>::const_iterator i(iuse.begin()), i_end(iuse.end()) ;
                    i != i_end ; ++i)
            {
                std::string::size_type delim_pos;
                if (std::string::npos == ((delim_pos = use_expand_delim_pos(*i,
                                    use_interface->use_expand_prefixes()))))
                    continue;
                if (use_interface->use_expand_hidden_prefixes()->count(UseFlagName(i->data().substr(
                                    0, delim_pos))))
                    continue;

                expand_name = UseFlagName(i->data().substr(0, delim_pos));
                expand_value = UseFlagName(i->data().substr(delim_pos + 1));

                if (expand_name != old_expand_name)
                {
                    if (! use_expand_string.empty())
                    {
                        Gtk::TreeModel::Row use_row = *(model->append(row->children()));
                        use_row[col_left] = stringify(old_expand_name) + ":";
                        use_row[col_right] = use_expand_string;
                    }

                    old_expand_name = expand_name;
                    use_expand_string = "";
                }

                if (! use_expand_string.empty())
                    use_expand_string.append(" ");

                if (env->query_use(*i, &p))
                {
                    if (use_interface && use_interface->query_use_force(*i, &p))
                        use_expand_string.append(pango_colour(cl_flag_on, "(" + stringify(expand_value) + ")"));
                    else
                        use_expand_string.append(pango_colour(cl_flag_on, expand_value));
                }
                else
                {
                    if (use_interface && use_interface->query_use_mask(*i, &p))
                        use_expand_string.append(pango_colour(cl_flag_off, "(-" + stringify(expand_value) + ")"));
                    else
                        use_expand_string.append(pango_colour(cl_flag_off, "-" + stringify(expand_value)));
                }

                if (other_p)
                    if (env->query_use(*i, &p) != env->query_use(*i, other_p))
                        use_expand_string.append("*");
            }

            if (! use_expand_string.empty())
            {
                Gtk::TreeModel::Row use_row = *(model->append(row->children()));
                use_row[col_left] = stringify(expand_name) + ":";
                use_row[col_right] = use_expand_string;
            }
        }

    }
}

namespace paludis
{
    template<>
    struct Implementation<QueueList>
    {
        QueuePage * const page;

        Columns columns;
        Glib::RefPtr<Gtk::TreeStore> model;
        OurInstallTask install_task;
        bool installed_signal;
        Glib::RefPtr<Gdk::Pixbuf> target_icon;

        Implementation(QueuePage * const p) :
            page(p),
            model(Gtk::TreeStore::create(columns)),
            installed_signal(false),
            target_icon(page->render_icon(Gtk::Stock::MISSING_IMAGE, Gtk::ICON_SIZE_MENU))
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
    append_column("", _imp->columns.col_icon);
    append_column("Target", _imp->columns.col_left);

    Glib::RefPtr<Gtk::TreeStore> new_model(Gtk::TreeStore::create(_imp->columns));

    for (InstallTask::TargetsIterator i(_imp->install_task.begin_targets()),
            i_end(_imp->install_task.end_targets()) ; i != i_end ; ++i)
    {
        Gtk::TreeModel::Row row = *(new_model->append());
        row[_imp->columns.col_icon] = _imp->target_icon;
        row[_imp->columns.col_left] = stringify(*i);
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
            Glib::RefPtr<Gtk::TreeStore> _model;

        public:
            Populate(QueueList * const q, Glib::RefPtr<Gtk::TreeStore> model) :
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
    if (e.kind != dlk_package)
        return;

    Gtk::TreeModel::Row row = *(_model->append());

    row[_q->_imp->columns.col_icon] = _q->_imp->target_icon;

    std::string left;
    if (e.package.repository == DefaultEnvironment::get_instance()->package_database()->favourite_repository())
        left = stringify(e.package.name);
    else
        left = stringify(e.package.name) + "::" + stringify(e.package.repository);

    if (e.metadata->slot != SlotName("0"))
        left.append(" :" + pango_colour("#0000cc", stringify(e.metadata->slot)));

    row[_q->_imp->columns.col_left] = left;

    std::tr1::shared_ptr<PackageDatabaseEntryCollection> existing(DefaultEnvironment::get_instance()->package_database()->
            query(PackageDepSpec(e.package.name), is_installed_only, qo_order_by_version));

    std::string right = stringify(e.package.version);

    if (existing->empty())
        right.append(" [N]");
    else
    {
        existing = DefaultEnvironment::get_instance()->package_database()->query(PackageDepSpec(
                    stringify(e.package.name) + ":" + stringify(e.metadata->slot)),
                is_installed_only, qo_order_by_version);
        if (existing->empty())
            right.append(" [S]");
        else if (existing->last()->version < e.package.version)
            right.append(" [U " + stringify(existing->last()->version) + "]");
        else if (existing->last()->version > e.package.version)
            right.append(" [D " + stringify(existing->last()->version) + "]");
        else
            right.append(" [R]");
    }
    row[_q->_imp->columns.col_right] = right;

    add_use_rows(_model, row, _q->_imp->columns.col_left, _q->_imp->columns.col_right,
            DefaultEnvironment::get_instance(), e.package, e.metadata,
            existing->empty() ? 0 : &*existing->last());
}

void
QueueList::calculate()
{
    PaludisThread::get_instance()->launch(std::tr1::shared_ptr<Populate>(new Populate(this,
                    Gtk::TreeStore::create(_imp->columns))));
}

void
QueueList::set_model_show_dep_columns(Glib::RefPtr<Gtk::TreeStore> new_model)
{
    remove_all_columns();
    append_column("", _imp->columns.col_icon);
    {
        Gtk::CellRendererText * const renderer(new Gtk::CellRendererText);

        Gtk::TreeViewColumn * column(new Gtk::TreeViewColumn("Package", *Gtk::manage(renderer)));
        column->add_attribute(renderer->property_markup(), _imp->columns.col_left);
        append_column(*column);
        set_expander_column(*column);

        column = new Gtk::TreeViewColumn("Details", *renderer);
        column->add_attribute(renderer->property_markup(), _imp->columns.col_right);
        append_column(*column);
    }

    _imp->model.swap(new_model);
    set_model(_imp->model);
    expand_all();

    _imp->page->set_queue_list_calculated(true);
}

