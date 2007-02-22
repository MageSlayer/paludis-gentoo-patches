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

#include "set_overview.hh"
#include "paludis_thread.hh"
#include "main_window.hh"

#include <paludis/environment/default/default_environment.hh>
#include <paludis/util/save.hh>
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
            Gtk::TreeModelColumn<Glib::ustring> col_spec;
            Gtk::TreeModelColumn<Glib::ustring> col_tag;

            Columns()
            {
                add(col_spec);
                add(col_tag);
            }
    };

    struct TagDecoder :
        public DepTagVisitorTypes::ConstVisitor
    {
        std::string text;

        void visit(const GLSADepTag * const tag)
        {
            text = tag->short_text() + ": " + tag->glsa_title();
        }

        void visit(const DependencyDepTag * const)
        {
        }

        void visit(const GeneralSetDepTag * const tag)
        {
            text = tag->short_text() + ": " + tag->source();
        }
    };

    class TreeFromDepSpec :
        private InstantiationPolicy<TreeFromDepSpec, instantiation_method::NonCopyableTag>,
        public DepSpecVisitorTypes::ConstVisitor
    {
        private:
            Glib::RefPtr<Gtk::TreeStore> _model;
            const Columns * const _columns;
            Gtk::TreeModel::Row * _row;

        public:
            TreeFromDepSpec(Glib::RefPtr<Gtk::TreeStore> model, const Columns * const columns,
                    Gtk::TreeModel::Row * top_row) :
                _model(model),
                _columns(columns),
                _row(top_row)
            {
            }

            void visit(const AllDepSpec * spec)
            {
                if (spec->begin() != spec->end())
                {
                    Gtk::TreeModel::Row new_row(*_model->append(_row->children()));
                    new_row[_columns->col_spec] = "all of";
                    Save<Gtk::TreeModel::Row *> save_row(&_row, &new_row);
                    std::for_each(spec->begin(), spec->end(), accept_visitor(this));
                }
            }

            void visit(const AnyDepSpec * spec)
            {
                if (spec->begin() != spec->end())
                {
                    Gtk::TreeModel::Row new_row(*_model->append(_row->children()));
                    new_row[_columns->col_spec] = "any of";
                    Save<Gtk::TreeModel::Row *> save_row(&_row, &new_row);
                    std::for_each(spec->begin(), spec->end(), accept_visitor(this));
                }
            }

            void visit(const UseDepSpec * spec)
            {
                Gtk::TreeModel::Row new_row(*_model->append(_row->children()));
                new_row[_columns->col_spec] = "if " + stringify(spec->inverse() ? "!" : "") + stringify(spec->flag());
                Save<Gtk::TreeModel::Row *> save_row(&_row, &new_row);
                std::for_each(spec->begin(), spec->end(), accept_visitor(this));
            }

            void visit(const PlainTextDepSpec * spec)
            {
                Gtk::TreeModel::Row new_row(*_model->append(_row->children()));
                new_row[_columns->col_spec] = spec->text();
            }

            void visit(const PackageDepSpec * spec)
            {
                Gtk::TreeModel::Row new_row(*_model->append(_row->children()));
                new_row[_columns->col_spec] = stringify(*spec);

                if (spec->tag())
                {
                    TagDecoder d;
                    spec->tag()->accept(&d);
                    new_row[_columns->col_tag] = d.text;
                }
            }

            void visit(const BlockDepSpec * spec)
            {
                Gtk::TreeModel::Row new_row(*_model->append(_row->children()));
                new_row[_columns->col_spec] = "!" + stringify(*spec->blocked_spec());
            }
    };
}

namespace paludis
{
    template<>
    struct Implementation<SetOverview>
    {
        SetOverview * const info;
        Columns columns;
        Glib::RefPtr<Gtk::TreeStore> model;

        Implementation(SetOverview * const o) :
            info(o),
            model(Gtk::TreeStore::create(columns))
        {
        }

        void set_model(const Glib::RefPtr<Gtk::TreeStore> & m)
        {
            model = m;
            info->set_model(model);
            info->expand_all();
            info->columns_autosize();
        }
    };
}

SetOverview::SetOverview() :
    PrivateImplementationPattern<SetOverview>(new Implementation<SetOverview>(this))
{
    set_model(_imp->model);
    append_column("Atom", _imp->columns.col_spec);
    append_column("Tag", _imp->columns.col_tag);
}

SetOverview::~SetOverview()
{
}

namespace
{
    class Populate :
        public PaludisThread::Launchable
    {
        private:
            Implementation<SetOverview> * const _imp;
            SetName _set;

        public:
            Populate(Implementation<SetOverview> * const imp, const std::string & set) :
                _imp(imp),
                _set(set)
            {
            }

            virtual void operator() ();
    };

    void
    Populate::operator() ()
    {
        Glib::RefPtr<Gtk::TreeStore> model(Gtk::TreeStore::create(_imp->columns));

        StatusBarMessage m1(this, "Querying set information...");

        std::tr1::shared_ptr<const DepSpec> set_spec(DefaultEnvironment::get_instance()->package_set(_set));
        Gtk::TreeModel::Row top_row = *model->append();
        top_row[_imp->columns.col_spec] = stringify(_set);
        TreeFromDepSpec v(model, &_imp->columns, &top_row);
        set_spec->accept(&v);

        dispatch(sigc::bind<1>(sigc::mem_fun(_imp, &Implementation<SetOverview>::set_model), model));
    }
}

void
SetOverview::populate(const std::string & name)
{
    if (name.empty())
        _imp->set_model(Gtk::TreeStore::create(_imp->columns));
    else
        PaludisThread::get_instance()->launch(std::tr1::shared_ptr<Populate>(new Populate(_imp.operator-> (), name)));
}



