/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "main_window.hh"
#include "markup.hh"
#include "packages_list_model.hh"
#include "packages_page.hh"
#include <paludis/dep_spec_flattener.hh>
#include <paludis/environment.hh>
#include <paludis/package_database.hh>
#include <paludis/metadata_key.hh>
#include <paludis/package_id.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/tr1_functional.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/options.hh>
#include <list>
#include <algorithm>
#include <set>

using namespace paludis;
using namespace gtkpaludis;

namespace paludis
{
    template<>
    struct Implementation<PackagesListModel>
    {
        MainWindow * const main_window;
        PackagesPage * const packages_page;
        PackagesListModel::Columns columns;

        Implementation(MainWindow * const m, PackagesPage * const p) :
            main_window(m),
            packages_page(p)
        {
        }
    };
}

namespace
{
    struct PopulateItem
    {
        std::string title;
        std::string status_markup;
        std::string description;
        tr1::shared_ptr<const QualifiedPackageName> qpn;
        PackagesPackageFilterOption local_best_option;
        std::list<PopulateItem> children;
        bool merge_if_one_child;

        const PackagesPackageFilterOption children_best_option() const
        {
            PackagesPackageFilterOption result(local_best_option);
            for (std::list<PopulateItem>::const_iterator i(children.begin()), i_end(children.end()) ;
                    i != i_end ; ++i)
                result = std::max(result, i->children_best_option());

            return result;
        }

        PopulateItem(const std::string & t) :
            title(t),
            local_best_option(ppfo_all_packages),
            merge_if_one_child(true)
        {
        }
    };
}

namespace gtkpaludis
{
    struct PackagesListModel::PopulateData
    {
        std::list<PopulateItem> items;
    };

    struct PackagesListModel::PopulateDataIterator :
        WrappedForwardIterator<PackagesListModel::PopulateDataIterator, const PopulateItem>
    {
        PopulateDataIterator(const std::list<PopulateItem>::const_iterator & i) :
            WrappedForwardIterator<PackagesListModel::PopulateDataIterator, const PopulateItem>(i)
        {
        }
    };
}


PackagesListModel::PackagesListModel(MainWindow * const m, PackagesPage * const p) :
    PrivateImplementationPattern<PackagesListModel>(new Implementation<PackagesListModel>(m, p)),
    Gtk::TreeStore(_imp->columns)
{
}

PackagesListModel::~PackagesListModel()
{
}

PackagesListModel::Columns::Columns()
{
    add(col_package);
    add(col_status_markup);
    add(col_description);
    add(col_qpn);
    add(col_best_package_filter_option);
}

PackagesListModel::Columns::~Columns()
{
}

PackagesListModel::Columns &
PackagesListModel::columns()
{
    return _imp->columns;
}

void
PackagesListModel::populate()
{
    _imp->main_window->paludis_thread_action(
            sigc::mem_fun(this, &PackagesListModel::populate_in_paludis_thread), "Populating packages list model");
}

namespace
{
    PopulateItem make_item(const PackageDepSpec & pds,
            const tr1::shared_ptr<const PackageID> & id,
            const Environment * const environment)
    {
        PackagesPackageFilterOption best_option(ppfo_all_packages);
        std::string status;
        paludis::tr1::shared_ptr<const PackageIDSequence> ci(
                environment->package_database()->query(
                    query::InstalledAtRoot(environment->root()) &
                    query::Matches(pds) &
                    query::Matches(make_package_dep_spec()
                        .package(id->name())
                        .slot_requirement(make_shared_ptr(new UserSlotExactRequirement(id->slot())))),
                    qo_order_by_version));

        paludis::tr1::shared_ptr<const PackageIDSequence> av(
                environment->package_database()->query(
                    query::SupportsAction<InstallAction>() &
                    query::Matches(pds) &
                    query::Matches(make_package_dep_spec()
                        .package(id->name())
                        .slot_requirement(make_shared_ptr(new UserSlotExactRequirement(id->slot())))) &
                    query::NotMasked(),
                    qo_order_by_version));

        if (! ci->empty())
        {
            status = markup_escape(((*ci->last())->canonical_form(idcf_version)));
            best_option = ppfo_installed_packages;

            if (! av->empty())
            {
                if ((*av->last())->version() < (*ci->last())->version())
                {
                    status.append(markup_bold(markup_escape(" > " + (*av->last())->canonical_form(idcf_version))));
                    best_option = ppfo_upgradable_packages;
                }
                else if ((*av->last())->version() > (*ci->last())->version())
                {
                    status.append(markup_bold(markup_escape(" < " + (*av->last())->canonical_form(idcf_version))));
                    best_option = ppfo_upgradable_packages;
                }
            }
        }
        else
        {
            paludis::tr1::shared_ptr<const PackageIDSequence> av(
                    environment->package_database()->query(
                        query::Matches(pds) &
                        query::Matches(make_package_dep_spec()
                            .package(id->name())
                            .slot_requirement(make_shared_ptr(new UserSlotExactRequirement(id->slot())))) &
                        query::SupportsAction<InstallAction>() &
                        query::NotMasked(),
                        qo_order_by_version));
            if (av->empty())
            {
                status.append(markup_foreground("grey", markup_escape("masked")));
                best_option = ppfo_all_packages;
            }
            else
            {
                status.append(markup_foreground("grey", markup_escape((*av->last())->canonical_form(idcf_version))));
                best_option = ppfo_visible_packages;
            }
        }

        PopulateItem result(stringify(id->slot()));
        result.status_markup = status;
        if (id->short_description_key())
            result.description = id->short_description_key()->value();
        else
            result.description = "(no description)";
        result.qpn = make_shared_ptr(new QualifiedPackageName(id->name()));
        result.local_best_option = best_option;
        return result;
    }
}

void
PackagesListModel::populate_in_paludis_thread()
{
    paludis::tr1::shared_ptr<PopulateData> data(new PopulateData);

    if (_imp->packages_page->get_category())
    {
        paludis::tr1::shared_ptr<const PackageIDSequence> c(
                _imp->main_window->environment()->package_database()->query(
                    *_imp->packages_page->get_repository_filter() &
                    query::Category(*_imp->packages_page->get_category()),
                    qo_best_version_in_slot_only));

        QualifiedPackageName old_qpn("OLD/OLD");

        for (PackageIDSequence::ReverseConstIterator p(c->rbegin()), p_end(c->rend()) ;
                p != p_end ; ++p)
        {
            if (old_qpn != (*p)->name())
            {
                data->items.push_front(PopulateItem(stringify((*p)->name().package)));
                data->items.begin()->children.push_front(make_item(make_package_dep_spec().package((*p)->name()),
                            *p, _imp->main_window->environment()));
                data->items.begin()->qpn = data->items.begin()->children.begin()->qpn;
                old_qpn = (*p)->name();
            }
            else
                data->items.begin()->children.push_front(make_item(
                            PackageDepSpec(make_package_dep_spec().package((*p)->name())),
                            *p, _imp->main_window->environment()));
        }
    }
    else if (_imp->packages_page->get_set())
    {
        DepSpecFlattener<SetSpecTree, PackageDepSpec> f(_imp->main_window->environment());
        _imp->main_window->environment()->set(*_imp->packages_page->get_set())->accept(f);
        std::set<std::string> a;
        std::transform(indirect_iterator(f.begin()), indirect_iterator(f.end()), std::inserter(a, a.begin()),
                tr1::mem_fn(&StringDepSpec::text));

        for (std::set<std::string>::const_iterator i(a.begin()), i_end(a.end()) ;
                i != i_end ; ++i)
        {
            std::list<PopulateItem>::iterator atom_iter(data->items.insert(data->items.end(), PopulateItem(*i)));
            atom_iter->merge_if_one_child = false;
            PackageDepSpec ds(parse_user_package_dep_spec(*i, UserPackageDepSpecOptions() + updso_allow_wildcards));
            if (ds.package_ptr())
            {
                paludis::tr1::shared_ptr<const PackageIDSequence> c(
                        _imp->main_window->environment()->package_database()->query(
                            *_imp->packages_page->get_repository_filter() &
                            query::Matches(ds),
                            qo_best_version_in_slot_only));

                for (PackageIDSequence::ReverseConstIterator p(c->rbegin()), p_end(c->rend()) ;
                        p != p_end ; ++p)
                {
                    atom_iter->children.push_back(make_item(ds,
                                *p, _imp->main_window->environment()));
                    atom_iter->qpn = atom_iter->children.back().qpn;
                }
            }
            else
            {
                paludis::tr1::shared_ptr<const PackageIDSequence> c(
                        _imp->main_window->environment()->package_database()->query(
                            *_imp->packages_page->get_repository_filter() &
                            query::Matches(ds),
                            qo_best_version_in_slot_only));

                QualifiedPackageName old_qpn("OLD/OLD");
                std::list<PopulateItem>::iterator pkg_iter;

                for (PackageIDSequence::ReverseConstIterator p(c->rbegin()), p_end(c->rend()) ;
                        p != p_end ; ++p)
                {
                    if (old_qpn != (*p)->name())
                    {
                        pkg_iter = atom_iter->children.insert(atom_iter->children.end(), PopulateItem(stringify((*p)->name())));
                        pkg_iter->children.push_back(make_item(ds,
                                    *p, _imp->main_window->environment()));
                        pkg_iter->qpn = pkg_iter->children.back().qpn;
                        old_qpn = (*p)->name();
                    }
                    else
                        pkg_iter->children.push_front(make_item(ds,
                                    *p, _imp->main_window->environment()));
                }
            }
        }
    }

    _imp->main_window->gui_thread_action(
            sigc::bind(sigc::mem_fun(this, &PackagesListModel::populate_in_gui_thread), data));
}

void
PackagesListModel::populate_in_gui_thread(paludis::tr1::shared_ptr<const PackagesListModel::PopulateData> names)
{
    clear();
    Gtk::TreeNodeChildren c(children());
    _populate_in_gui_thread_recursive(c,
            PopulateDataIterator(names->items.begin()),
            PopulateDataIterator(names->items.end()));
}

void
PackagesListModel::_populate_in_gui_thread_recursive(
        Gtk::TreeNodeChildren & t,
        PopulateDataIterator i,
        PopulateDataIterator i_end)
{
    for ( ; i != i_end ; ++i)
    {
        iterator r(append(t));
        (*r)[_imp->columns.col_package] = i->title;
        (*r)[_imp->columns.col_best_package_filter_option] = i->children_best_option();
        (*r)[_imp->columns.col_qpn] = i->qpn;
        (*r)[_imp->columns.col_description] = i->description;
        (*r)[_imp->columns.col_status_markup] = i->status_markup;

        if (! i->children.empty())
        {
            if (i->merge_if_one_child && (next(i->children.begin()) == i->children.end()))
            {
                (*r)[_imp->columns.col_description] = i->children.begin()->description;
                (*r)[_imp->columns.col_status_markup] = i->children.begin()->status_markup;

            }
            else
            {
                Gtk::TreeNodeChildren c(r->children());
                _populate_in_gui_thread_recursive(c, PopulateDataIterator(i->children.begin()),
                        PopulateDataIterator(i->children.end()));
            }
        }
    }
}

