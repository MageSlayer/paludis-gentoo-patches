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
#include <paludis/util/tokeniser.hh>
#include <paludis/environment/default/default_environment.hh>
#include <list>
#include <set>

#ifdef USE_BROKEN_CELL_RENDERER_BUTTON
#  include <cellrendererbutton/cellrendererbutton.hh>
#endif

using namespace paludis;
using namespace gtkpaludis;

namespace
{
    class Columns :
        public Gtk::TreeModel::ColumnRecord
    {
        public:
            Gtk::TreeModelColumn<Glib::ustring> col_package;
            Gtk::TreeModelColumn<Glib::ustring> col_slot;
            Gtk::TreeModelColumn<Glib::ustring> col_status;
            Gtk::TreeModelColumn<Glib::ustring> col_use;
            Gtk::TreeModelColumn<Glib::ustring> col_tags;
#ifdef USE_BROKEN_CELL_RENDERER_BUTTON
            Gtk::TreeModelColumn<Glib::ustring> col_why;
#endif

            Columns()
            {
                add(col_package);
                add(col_slot);
                add(col_status);
                add(col_use);
                add(col_tags);
#ifdef USE_BROKEN_CELL_RENDERER_BUTTON
                add(col_why);
#endif
            }
    };

    template <typename T_>
    std::string
    pango_colour(const std::string & c, const T_ & s)
    {
        return "<span foreground=\"" + c + "\">" + stringify(s) + "</span>";
    }

    std::string::size_type
    use_expand_delim_pos(const UseFlagName & u, const UseFlagNameCollection::ConstPointer c)
    {
        for (UseFlagNameCollection::Iterator i(c->begin()), i_end(c->end()) ; i != i_end ; ++i)
            if (0 == u.data().compare(0, i->data().length(), i->data(), 0, i->data().length()))
                return i->data().length();
        return std::string::npos;
    }

    std::string
    make_pretty_use_flags_string(const Environment * const env, const PackageDatabaseEntry & p,
            VersionMetadata::ConstPointer metadata, const PackageDatabaseEntry * const other_p)
    {
        static const std::string cl_flag_on("#00cc00");
        static const std::string cl_flag_off("#cc0000");

        std::ostringstream c;

        if (metadata->get_ebuild_interface())
        {
            const RepositoryUseInterface * const use_interface(
                    env->package_database()->
                    fetch_repository(p.repository)->use_interface);
            std::set<UseFlagName> iuse;
            WhitespaceTokeniser::get_instance()->tokenise(
                    metadata->get_ebuild_interface()->iuse,
                    create_inserter<UseFlagName>(std::inserter(iuse, iuse.end())));

            /* display normal use flags first */
            for (std::set<UseFlagName>::const_iterator i(iuse.begin()), i_end(iuse.end()) ;
                    i != i_end ; ++i)
            {
                if (std::string::npos != use_expand_delim_pos(*i, use_interface->use_expand_prefixes()))
                    continue;

                if (env->query_use(*i, &p))
                {
                    if (use_interface && use_interface->query_use_force(*i, &p))
                        c << " " << pango_colour(cl_flag_on, "(" + stringify(*i) + ")");
                    else
                        c << " " << pango_colour(cl_flag_on, *i);
                }
                else
                {
                    if (use_interface && use_interface->query_use_mask(*i, &p))
                        c << " " << pango_colour(cl_flag_off, "(-" + stringify(*i) + ")");
                    else
                        c << " " << pango_colour(cl_flag_off, "-" + stringify(*i));
                }

                if (other_p)
                    if (env->query_use(*i, &p) != env->query_use(*i, other_p))
                        c << "*";
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

                UseFlagName expand_name(i->data().substr(0, delim_pos)),
                            expand_value(i->data().substr(delim_pos + 1));

                if (expand_name != old_expand_name)
                {
                    c << " " << expand_name << ":";
                    old_expand_name = expand_name;
                }

                if (env->query_use(*i, &p))
                {
                    if (use_interface && use_interface->query_use_force(*i, &p))
                        c << " " << pango_colour(cl_flag_on, "(" + stringify(expand_value) + ")");
                    else
                        c << " " << pango_colour(cl_flag_on, expand_value);
                }
                else
                {
                    if (use_interface && use_interface->query_use_mask(*i, &p))
                        c << " " << pango_colour(cl_flag_off, "(-" + stringify(expand_value) + ")");
                    else
                        c << " " << pango_colour(cl_flag_off, "-" + stringify(expand_value));
                }

                if (other_p)
                    if (env->query_use(*i, &p) != env->query_use(*i, other_p))
                        c << "*";
            }
        }

        return c.str();
    }
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
    if (e.skip_install)
        return;

    Gtk::TreeModel::Row row = *(_model->append());

    if (e.package.repository == DefaultEnvironment::get_instance()->package_database()->favourite_repository())
        row[_q->_imp->columns.col_package] = stringify(e.package.name) + "-" +
            stringify(e.package.version);
    else
        row[_q->_imp->columns.col_package] = stringify(e.package);

    PackageDatabaseEntryCollection::Pointer existing(DefaultEnvironment::get_instance()->package_database()->
            query(PackageDepAtom::Pointer(new PackageDepAtom(stringify(
                            e.package.name))), is_installed_only));

    if (existing->empty())
        row[_q->_imp->columns.col_status] = "N";
    else
    {
        existing = DefaultEnvironment::get_instance()->package_database()->query(PackageDepAtom::Pointer(
                    new PackageDepAtom(stringify(e.package.name) + ":" +
                        stringify(e.metadata->slot))),
                is_installed_only);
        if (existing->empty())
            row[_q->_imp->columns.col_status] = "S";
        else if (existing->last()->version < e.package.version)
            row[_q->_imp->columns.col_status] = "U " + stringify(existing->last()->version);
        else if (existing->last()->version > e.package.version)
            row[_q->_imp->columns.col_status] = "D " + stringify(existing->last()->version);
        else
            row[_q->_imp->columns.col_status] = "R";
    }

    row[_q->_imp->columns.col_slot] = ":" + stringify(e.metadata->slot);
    row[_q->_imp->columns.col_use] = make_pretty_use_flags_string(DefaultEnvironment::get_instance(),
            e.package, e.metadata, 0);

#ifdef USE_BROKEN_CELL_RENDERER_BUTTON
    row[_q->_imp->columns.col_why] = " ... ";
#endif
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
    get_column(append_column("Package", _imp->columns.col_package) - 1)->set_expand(true);
    append_column("Slot", _imp->columns.col_slot);
    append_column("Status", _imp->columns.col_status);
    {
        Gtk::CellRendererText * const renderer(new Gtk::CellRendererText);
        Gtk::TreeViewColumn * const column(new Gtk::TreeViewColumn("Use", *Gtk::manage(renderer)));
        column->add_attribute(renderer->property_markup(), _imp->columns.col_use);
        append_column(*column);
    }

    append_column("Tags", _imp->columns.col_tags);

#ifdef USE_BROKEN_CELL_RENDERER_BUTTON
    {
        CellRendererButton * const renderer = new CellRendererButton(*this);
        renderer->property_text_x_pad() = 0;
        renderer->property_text_y_pad() = 0;

        Gtk::TreeViewColumn * const column = new Gtk::TreeViewColumn("Why",
                *Gtk::manage(renderer));
        column->add_attribute(renderer->property_text(), _imp->columns.col_why);
        renderer->set_column(column);
        append_column(*column);
    }
#endif

    _imp->model.swap(new_model);
    set_model(_imp->model);

    _imp->page->set_queue_list_calculated(true);
}

