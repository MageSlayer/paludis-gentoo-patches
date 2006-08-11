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

#include "browse_tree.hh"
#include "information_tree.hh"
#include "main_window.hh"
#include "sync.hh"
#include "vtemm/reaper.hh"

#include <gtkmm/treestore.h>
#include <gtkmm/menu.h>
#include <gtkmm/messagedialog.h>

#include <paludis/default_environment.hh>
#include <paludis/util/log.hh>
#include <paludis/syncer.hh>

#include <sys/types.h>
#include <sys/wait.h>
#include <iostream>

using namespace paludis;

namespace
{
    class BrowseTreeColumns;

    class BrowseTreeDisplayData :
        public InternalCounted<BrowseTreeDisplayData>
    {
        private:
            bool _has_children;

        protected:
            BrowseTreeDisplayData();

            virtual void load_children(
                    Glib::RefPtr<Gtk::TreeStore>,
                    const BrowseTreeColumns & columns,
                    Gtk::TreeModel::iterator) const = 0;

        public:
            virtual ~BrowseTreeDisplayData();

            virtual void display(InformationTree * const information_tree) const = 0;

            virtual void sync() const
            {
            }

            virtual void need_children(
                    Glib::RefPtr<Gtk::TreeStore> model,
                    const BrowseTreeColumns & columns,
                    Gtk::TreeModel::iterator us)
            {
                if (! _has_children)
                {
                    load_children(model, columns, us);
                    _has_children = true;
                }
            }

            class DefaultablePointer :
                public Pointer
            {
                public:
                    DefaultablePointer() :
                        Pointer(0)
                    {
                    }

                    template <typename T_>
                    DefaultablePointer(T_ v) :
                        Pointer(v)
                    {
                    }
            };
    };

    class BrowseTreeColumns :
        public Gtk::TreeModel::ColumnRecord
    {
        public:
            BrowseTreeColumns();
            ~BrowseTreeColumns();

            Gtk::TreeModelColumn<Glib::ustring> col_item;
            Gtk::TreeModelColumn<BrowseTreeDisplayData::DefaultablePointer> col_data;
    };

    class BrowseTreeDisplayTopData :
        public BrowseTreeDisplayData
    {
        protected:
            virtual void load_children(
                    Glib::RefPtr<Gtk::TreeStore>,
                    const BrowseTreeColumns &,
                    Gtk::TreeModel::iterator) const
            {
            }

        public:
            virtual void display(InformationTree * const information_tree) const
            {
                information_tree->show_top();
            }
    };

    class BrowseTreeDisplayVersionData :
        public BrowseTreeDisplayData
    {
        private:
            RepositoryName _r;
            QualifiedPackageName _q;
            VersionSpec _v;

        protected:
            virtual void load_children(
                    Glib::RefPtr<Gtk::TreeStore>,
                    const BrowseTreeColumns &,
                    Gtk::TreeModel::iterator) const
            {
            }

        public:
            BrowseTreeDisplayVersionData(const RepositoryName & r,
                    const QualifiedPackageName q, const VersionSpec & v) :
                _r(r),
                _q(q),
                _v(v)
            {
            }

            virtual void display(InformationTree * const information_tree) const
            {
                information_tree->show_version(_r, _q, _v);
            }
    };

    class BrowseTreeDisplayPackageData :
        public BrowseTreeDisplayData
    {
        private:
            RepositoryName _r;
            QualifiedPackageName _q;

        protected:
            virtual void load_children(
                    Glib::RefPtr<Gtk::TreeStore> model,
                    const BrowseTreeColumns & columns,
                    Gtk::TreeModel::iterator us) const
            {
                Repository::ConstPointer repo(
                        DefaultEnvironment::get_instance()->package_database()->fetch_repository(_r));

                VersionSpecCollection::ConstPointer vers(repo->version_specs(_q));
                for (VersionSpecCollection::Iterator ver(vers->begin()), ver_end(vers->end()) ;
                        ver != ver_end ; ++ver)
                {
                    Gtk::TreeModel::Row ver_row = *(model->append(us->children()));
                    ver_row[columns.col_item] = stringify(*ver);
                    ver_row[columns.col_data] = BrowseTreeDisplayData::Pointer(
                            new BrowseTreeDisplayVersionData(_r, _q, *ver));
                }
            }

        public:
            BrowseTreeDisplayPackageData(const RepositoryName & r,
                    const QualifiedPackageName q) :
                _r(r),
                _q(q)
            {
            }

            virtual void display(InformationTree * const information_tree) const
            {
                information_tree->show_package(_r, _q);
            }
    };

    class BrowseTreeDisplayCategoryData :
        public BrowseTreeDisplayData
    {
        private:
            RepositoryName _r;
            CategoryNamePart _c;

        protected:
            virtual void load_children(
                    Glib::RefPtr<Gtk::TreeStore> model,
                    const BrowseTreeColumns & columns,
                    Gtk::TreeModel::iterator us) const
            {
                Repository::ConstPointer repo(
                        DefaultEnvironment::get_instance()->package_database()->fetch_repository(_r));

                QualifiedPackageNameCollection::ConstPointer pkgs(repo->package_names(_c));
                for (QualifiedPackageNameCollection::Iterator pkg(pkgs->begin()), pkg_end(pkgs->end()) ;
                        pkg != pkg_end ; ++pkg)
                {
                    Gtk::TreeModel::Row pkg_row = *(model->append(us->children()));
                    pkg_row[columns.col_item] = stringify(pkg->package);
                    pkg_row[columns.col_data] = BrowseTreeDisplayData::Pointer(
                            new BrowseTreeDisplayPackageData(_r, *pkg));
                }
            }

        public:
            BrowseTreeDisplayCategoryData(const RepositoryName & r,
                    const CategoryNamePart c) :
                _r(r),
                _c(c)
            {
            }

            virtual void display(InformationTree * const information_tree) const
            {
                information_tree->show_category(_r, _c);
            }
    };

    class BrowseTreeDisplayRepositoryData :
        public BrowseTreeDisplayData
    {
        private:
            RepositoryName _r;

        protected:
            virtual void load_children(
                    Glib::RefPtr<Gtk::TreeStore> model,
                    const BrowseTreeColumns & columns,
                    Gtk::TreeModel::iterator us) const
            {
                Repository::ConstPointer repo(
                        DefaultEnvironment::get_instance()->package_database()->fetch_repository(_r));

                CategoryNamePartCollection::ConstPointer cats(repo->category_names());
                for (CategoryNamePartCollection::Iterator cat(cats->begin()), cat_end(cats->end()) ;
                        cat != cat_end ; ++cat)
                {
                    Gtk::TreeModel::Row cat_row = *(model->append(us->children()));
                    cat_row[columns.col_item] = stringify(*cat);
                    cat_row[columns.col_data] = BrowseTreeDisplayData::Pointer(
                            new BrowseTreeDisplayCategoryData(_r, *cat));
                }
            }

        public:
            BrowseTreeDisplayRepositoryData(const RepositoryName & r) :
                _r(r)
            {
            }

            virtual void display(InformationTree * const information_tree) const
            {
                information_tree->show_repository(_r);
            }

            virtual void sync() const
            {
                OurSyncTask task;
                task.add_target(stringify(_r));
                task.execute();
            }

    };
}

BrowseTreeDisplayData::BrowseTreeDisplayData() :
    _has_children(false)
{
}

BrowseTreeDisplayData::~BrowseTreeDisplayData()
{
}


BrowseTreeColumns::BrowseTreeColumns()
{
    add(col_item);
    add(col_data);
}

BrowseTreeColumns::~BrowseTreeColumns()
{
}

namespace paludis
{
    template<>
    struct Implementation<BrowseTree> :
        InternalCounted<Implementation<BrowseTree> >
    {
        MainWindow * const main_window;
        InformationTree * const information_tree;

        BrowseTreeColumns columns;
        Glib::RefPtr<Gtk::TreeStore> model;

        Gtk::Menu popup_menu;

        pid_t paludis_child;

        Implementation(MainWindow * const m, InformationTree * const i) :
            main_window(m),
            information_tree(i),
            paludis_child(-1)
        {
        }

        virtual ~Implementation()
        {
        }
    };
}

BrowseTree::BrowseTree(MainWindow * const main_window,
        InformationTree * const information_tree) :
    PrivateImplementationPattern<BrowseTree>(new Implementation<BrowseTree>(
                main_window, information_tree))
{
    _imp->model = Gtk::TreeStore::create(_imp->columns);
    set_model(_imp->model);

    Gtk::TreeModel::Row repositories_row = *(_imp->model->append());
    repositories_row[_imp->columns.col_item] = "Repositories";
    repositories_row[_imp->columns.col_data] = BrowseTreeDisplayData::DefaultablePointer(
            new BrowseTreeDisplayTopData);

    for (PackageDatabase::RepositoryIterator
            r(DefaultEnvironment::get_instance()->package_database()->begin_repositories()),
            r_end(DefaultEnvironment::get_instance()->package_database()->end_repositories()) ;
            r != r_end ; ++r)
    {
        Gtk::TreeModel::Row repository_row = *(_imp->model->append(repositories_row.children()));
        repository_row[_imp->columns.col_item] = stringify((*r)->name());
        repository_row[_imp->columns.col_data] = BrowseTreeDisplayData::Pointer(
                new BrowseTreeDisplayRepositoryData((*r)->name()));
    }

    append_column("Item", _imp->columns.col_item);
    get_selection()->signal_changed().connect(sigc::mem_fun(*this, &BrowseTree::on_changed));
    Vte::Reaper::get_instance()->signal_child_exited().connect(sigc::mem_fun(*this, &BrowseTree::on_child_process_exited));

    /* popup menu */
    _imp->popup_menu.items().push_back(Gtk::Menu_Helpers::MenuElem("_Sync",
                sigc::mem_fun(*this, &BrowseTree::on_menu_sync)));
    _imp->popup_menu.accelerate(*this);
}

BrowseTree::~BrowseTree()
{
}

void
BrowseTree::on_changed()
{
    Gtk::TreeModel::iterator i(get_selection()->get_selected());
    if (i)
    {
        Gtk::TreeModel::Row row(*i);
        BrowseTreeDisplayData::Pointer(row[_imp->columns.col_data])->need_children(_imp->model,
                _imp->columns, i);
        BrowseTreeDisplayData::Pointer(row[_imp->columns.col_data])->display(
                _imp->information_tree);
    }
}

void
BrowseTree::on_menu_sync()
{
    Glib::RefPtr<Gtk::TreeView::Selection> selection(get_selection());
    if (selection)
    {
        Gtk::TreeModel::iterator i(selection->get_selected());
        if (i)
        {
            _imp->main_window->set_children_sensitive(false);

            pid_t child(fork());
            if (0 == child)
            {
                try
                {
                    BrowseTreeDisplayData::Pointer((*i)[_imp->columns.col_data])->sync();
                    _exit(0);
                }
                catch (const SyncFailedError & e)
                {
                    _exit(1);
                }
            }
            else if (-1 == child)
                throw InternalError(PALUDIS_HERE, "fork failed");
            else
            {
                Log::get_instance()->message(ll_debug, lc_no_context,
                        "Forked child process " + stringify(child));
                _imp->paludis_child = child;
                Vte::Reaper::get_instance()->add_child(child);
            }
        }
    }
}

bool
BrowseTree::on_button_press_event(GdkEventButton * event)
{
    bool result(TreeView::on_button_press_event(event));

    if (event->type == GDK_BUTTON_PRESS && event->button == 3)
        _imp->popup_menu.popup(event->button, event->time);

    return result;
}

void
BrowseTree::on_child_process_exited(int, int status)
{
    if (-1 == _imp->paludis_child)
        return;

    if (0 == status)
        Log::get_instance()->message(ll_debug, lc_no_context, "child " + stringify(_imp->paludis_child)
                 + " exited with success");
    else
        Log::get_instance()->message(ll_debug, lc_no_context, "child " + stringify(_imp->paludis_child)
                 + " exited with failure code " + stringify(status));

    _imp->paludis_child = -1;
    _imp->main_window->set_children_sensitive(true);
}

