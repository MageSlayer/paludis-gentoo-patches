/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "packages_list_model.hh"
#include "main_window.hh"
#include "packages_page.hh"
#include "markup.hh"
#include <paludis/util/collection_concrete.hh>
#include <paludis/util/iterator.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/environment.hh>
#include <paludis/package_database.hh>
#include <list>

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
    struct PopulateDataSubItem
    {
        std::string slot;
        std::string status;
        std::string description;
        PackageDatabaseEntry pde;

        PopulateDataSubItem(const std::string & s, const std::string & t, const std::string & d,
                const PackageDatabaseEntry p) :
            slot(s),
            status(t),
            description(d),
            pde(p)
        {
        }
    };

    struct PopulateDataItem
    {
        std::string package;
        PackagesPackageFilterOption best_option;
        std::list<PopulateDataSubItem> subitems;

        PopulateDataItem(const std::string & p) :
            package(p),
            best_option(ppfo_all_packages)
        {
        }
    };
}

namespace gtkpaludis
{
    struct PackagesListModel::PopulateData
    {
        std::list<PopulateDataItem> items;
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
    add(col_pde);
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
    PopulateDataSubItem make_item(const PackageDatabaseEntry & pde,
            paludis::tr1::shared_ptr<const VersionMetadata> metadata,
            const Environment * const environment,
            PackagesPackageFilterOption * const best_option)
    {
        std::string status;
        paludis::tr1::shared_ptr<const PackageDatabaseEntryCollection> ci(
                environment->package_database()->query(
                    query::Matches(PackageDepSpec(
                            paludis::tr1::shared_ptr<QualifiedPackageName>(new QualifiedPackageName(pde.name)),
                            paludis::tr1::shared_ptr<CategoryNamePart>(),
                            paludis::tr1::shared_ptr<PackageNamePart>(),
                            paludis::tr1::shared_ptr<VersionRequirements>(),
                            vr_and,
                            paludis::tr1::shared_ptr<SlotName>(new SlotName(metadata->slot)))) &
                    query::InstalledAtRoot(environment->root()),
                    qo_order_by_version));

        paludis::tr1::shared_ptr<const PackageDatabaseEntryCollection> av(
                environment->package_database()->query(
                    query::Matches(PackageDepSpec(
                            paludis::tr1::shared_ptr<QualifiedPackageName>(new QualifiedPackageName(pde.name)),
                            paludis::tr1::shared_ptr<CategoryNamePart>(),
                            paludis::tr1::shared_ptr<PackageNamePart>(),
                            paludis::tr1::shared_ptr<VersionRequirements>(),
                            vr_and,
                            paludis::tr1::shared_ptr<SlotName>(new SlotName(metadata->slot)))) &
                    query::RepositoryHasInstallableInterface() &
                    query::NotMasked(),
                    qo_order_by_version));

        if (! ci->empty())
        {
            status = markup_escape(stringify(ci->last()->version));
            *best_option = std::max(*best_option, ppfo_installed_packages);

            if (! av->empty())
            {
                if (av->last()->version < ci->last()->version)
                {
                    status.append(markup_bold(markup_escape(" > " + stringify(av->last()->version))));
                    *best_option = std::max(*best_option, ppfo_upgradable_packages);
                }
                else if (av->last()->version > ci->last()->version)
                {
                    status.append(markup_bold(markup_escape(" < " + stringify(av->last()->version))));
                    *best_option = std::max(*best_option, ppfo_upgradable_packages);
                }
            }
        }
        else
        {
            paludis::tr1::shared_ptr<const PackageDatabaseEntryCollection> av(
                    environment->package_database()->query(
                        query::Matches(PackageDepSpec(
                                paludis::tr1::shared_ptr<QualifiedPackageName>(new QualifiedPackageName(pde.name)),
                                paludis::tr1::shared_ptr<CategoryNamePart>(),
                                paludis::tr1::shared_ptr<PackageNamePart>(),
                                paludis::tr1::shared_ptr<VersionRequirements>(),
                                vr_and,
                                paludis::tr1::shared_ptr<SlotName>(new SlotName(metadata->slot)))) &
                        query::RepositoryHasInstallableInterface() &
                        query::NotMasked(),
                        qo_order_by_version));
            if (av->empty())
            {
                status.append(markup_foreground("grey", markup_escape("masked")));
                *best_option = std::max(*best_option, ppfo_all_packages);
            }
            else
            {
                status.append(markup_foreground("grey", markup_escape(stringify(av->last()->version))));
                *best_option = std::max(*best_option, ppfo_visible_packages);
            }
        }

        return PopulateDataSubItem(
                stringify(metadata->slot),
                status,
                metadata->description,
                pde);
    }
}

void
PackagesListModel::populate_in_paludis_thread()
{
    paludis::tr1::shared_ptr<PopulateData> data(new PopulateData);
    paludis::tr1::shared_ptr<const PackageDatabaseEntryCollection> c;

    if (_imp->packages_page->get_category())
        c = _imp->main_window->environment()->package_database()->query(
                *_imp->packages_page->get_repository_filter() &
                query::Category(*_imp->packages_page->get_category()),
                qo_best_version_in_slot_only);
    else
        c.reset(new PackageDatabaseEntryCollection::Concrete);

    QualifiedPackageName old_qpn("OLD/OLD");
    PackagesPackageFilterOption * best_option(0);

    for (PackageDatabaseEntryCollection::ReverseIterator p(c->rbegin()), p_end(c->rend()) ;
            p != p_end ; ++p)
    {
        paludis::tr1::shared_ptr<const VersionMetadata> metadata(
                _imp->main_window->environment()->package_database()->fetch_repository(p->repository)->version_metadata(
                    p->name, p->version));

        if (old_qpn != p->name)
        {
            best_option = &data->items.insert(data->items.begin(), PopulateDataItem(stringify(p->name.package)))->best_option;
            data->items.begin()->subitems.push_front(make_item(*p, metadata, _imp->main_window->environment(),
                        best_option));
            old_qpn = p->name;
        }
        else
            data->items.begin()->subitems.push_front(make_item(*p, metadata, _imp->main_window->environment(),
                        best_option));
    }

    _imp->main_window->gui_thread_action(
            sigc::bind(sigc::mem_fun(this, &PackagesListModel::populate_in_gui_thread), data));
}

void
PackagesListModel::populate_in_gui_thread(paludis::tr1::shared_ptr<const PackagesListModel::PopulateData> names)
{
    clear();

    for (std::list<PopulateDataItem>::const_iterator i(names->items.begin()), i_end(names->items.end()) ;
            i != i_end ; ++i)
    {
        if (i->subitems.empty())
            continue;

        iterator r(append());
        (*r)[_imp->columns.col_package] = i->package;
        (*r)[_imp->columns.col_pde] = paludis::tr1::shared_ptr<PackageDatabaseEntry>(
                new PackageDatabaseEntry(i->subitems.begin()->pde));
        (*r)[_imp->columns.col_best_package_filter_option] = i->best_option;

        if (next(i->subitems.begin()) == i->subitems.end())
        {
            (*r)[_imp->columns.col_status_markup] = i->subitems.begin()->status;
            (*r)[_imp->columns.col_description] = i->subitems.begin()->description;
        }
        else
        {
            for (std::list<PopulateDataSubItem>::const_iterator j(i->subitems.begin()), j_end(i->subitems.end()) ;
                    j != j_end ; ++j)
            {
                iterator s(append(r->children()));
                (*s)[_imp->columns.col_package] = ":" + j->slot;
                (*s)[_imp->columns.col_status_markup] = j->status;
                (*s)[_imp->columns.col_description] = j->description;
                (*s)[_imp->columns.col_pde] = paludis::tr1::shared_ptr<PackageDatabaseEntry>(new PackageDatabaseEntry(j->pde));
                (*s)[_imp->columns.col_best_package_filter_option] = i->best_option;
            }
        }
    }
}


