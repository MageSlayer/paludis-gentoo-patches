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

#include <paludis/repositories/virtuals/virtuals_repository.hh>
#include <paludis/repositories/virtuals/package_id.hh>

#include <paludis/environment.hh>
#include <paludis/hashed_containers.hh>
#include <paludis/match_package.hh>
#include <paludis/package_database.hh>
#include <paludis/query.hh>
#include <paludis/repository_info.hh>
#include <paludis/action.hh>

#include <paludis/util/log.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/operators.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/tr1_functional.hh>
#include <paludis/util/map.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/mutex.hh>

#include <vector>
#include <utility>

#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <libwrapiter/libwrapiter_output_iterator.hh>

using namespace paludis;

typedef MakeHashedMap<QualifiedPackageName, tr1::shared_ptr<PackageIDSequence> >::Type IDMap;

namespace paludis
{
    template<>
    struct Implementation<VirtualsRepository>
    {
        const Environment * const env;

        mutable Mutex big_nasty_mutex;

        mutable std::vector<std::pair<QualifiedPackageName, tr1::shared_ptr<const PackageDepSpec> > > names;
        mutable bool has_names;

        mutable IDMap ids;
        mutable bool has_ids;

        Implementation(const Environment * const e) :
            env(e),
            has_names(false),
            has_ids(false)
        {
        }
    };
}

namespace
{
    struct NamesCategoryComparator
    {
        bool
        operator() (const std::pair<QualifiedPackageName, tr1::shared_ptr<const PackageDepSpec> > & a,
                const std::pair<QualifiedPackageName, tr1::shared_ptr<const PackageDepSpec> > & b) const
        {
            return a.first.category < b.first.category;
        }
    };

    struct NamesNameComparator
    {
        bool
        operator() (const std::pair<QualifiedPackageName, tr1::shared_ptr<const PackageDepSpec> > & a,
                const std::pair<QualifiedPackageName, tr1::shared_ptr<const PackageDepSpec> > & b) const
        {
            return a.first < b.first;
        }
    };
}

VirtualsRepository::VirtualsRepository(const Environment * const env) :
    Repository(RepositoryName("virtuals"), RepositoryCapabilities::create()
            .installed_interface(0)
            .use_interface(0)
            .sets_interface(0)
            .syncable_interface(0)
            .mirrors_interface(0)
            .environment_variable_interface(0)
            .world_interface(0)
            .provides_interface(0)
            .virtuals_interface(0)
            .destination_interface(0)
            .licenses_interface(0)
            .e_interface(0)
            .make_virtuals_interface(this)
            .qa_interface(0)
            .hook_interface(0)
            .manifest_interface(0),
            "virtuals"),
    PrivateImplementationPattern<VirtualsRepository>(
            new Implementation<VirtualsRepository>(env))
{
    tr1::shared_ptr<RepositoryInfoSection> config_info(new RepositoryInfoSection("Configuration information"));
    config_info->add_kv("format", "virtuals");
    _info->add_section(config_info);
}

VirtualsRepository::~VirtualsRepository()
{
}

void
VirtualsRepository::need_names() const
{
    Lock l(_imp->big_nasty_mutex);

    if (_imp->has_names)
        return;

    Context context("When loading names for virtuals repository:");

    Log::get_instance()->message(ll_debug, lc_context, "VirtualsRepository need_names");

    /* Determine our virtual name -> package mappings. */
    for (PackageDatabase::RepositoryIterator r(_imp->env->package_database()->begin_repositories()),
            r_end(_imp->env->package_database()->end_repositories()) ; r != r_end ; ++r)
    {
        if (! (*r)->provides_interface)
            continue;

        tr1::shared_ptr<const RepositoryProvidesInterface::ProvidesSequence> provides(
                (*r)->provides_interface->provided_packages());
        for (RepositoryProvidesInterface::ProvidesSequence::Iterator p(provides->begin()),
                p_end(provides->end()) ; p != p_end ; ++p)
            _imp->names.push_back(std::make_pair(p->virtual_name, tr1::shared_ptr<const PackageDepSpec>(
                            new PackageDepSpec(
                                tr1::shared_ptr<QualifiedPackageName>(new QualifiedPackageName(p->provided_by->name()))))));
    }

    std::sort(_imp->names.begin(), _imp->names.end(), NamesNameComparator());

    std::vector<std::pair<QualifiedPackageName, tr1::shared_ptr<const PackageDepSpec> > > new_names;

    for (PackageDatabase::RepositoryIterator r(_imp->env->package_database()->begin_repositories()),
            r_end(_imp->env->package_database()->end_repositories()) ; r != r_end ; ++r)
    {
        if (! (*r)->virtuals_interface)
            continue;

        tr1::shared_ptr<const RepositoryVirtualsInterface::VirtualsSequence> virtuals(
                (*r)->virtuals_interface->virtual_packages());
        for (RepositoryVirtualsInterface::VirtualsSequence::Iterator v(virtuals->begin()),
                v_end(virtuals->end()) ; v != v_end ; ++v)
        {
            std::pair<
                std::vector<std::pair<QualifiedPackageName, tr1::shared_ptr<const PackageDepSpec> > >::const_iterator,
                std::vector<std::pair<QualifiedPackageName, tr1::shared_ptr<const PackageDepSpec> > >::const_iterator> p(
                        std::equal_range(_imp->names.begin(), _imp->names.end(),
                            std::make_pair(v->virtual_name, tr1::shared_ptr<const PackageDepSpec>()),
                            NamesNameComparator()));

            if (p.first == p.second)
                new_names.push_back(std::make_pair(v->virtual_name, v->provided_by_spec));
        }
    }

    std::copy(new_names.begin(), new_names.end(), std::back_inserter(_imp->names));
    std::sort(_imp->names.begin(), _imp->names.end(), NamesNameComparator());

    _imp->has_names = true;
}

void
VirtualsRepository::need_ids() const
{
    Lock l(_imp->big_nasty_mutex);

    if (_imp->has_ids)
        return;

    Context context("When loading entries for virtuals repository:");
    need_names();

    Log::get_instance()->message(ll_debug, lc_context, "VirtualsRepository need_entries");

    /* Populate our _imp->entries. */
    for (std::vector<std::pair<QualifiedPackageName, tr1::shared_ptr<const PackageDepSpec> > >::const_iterator
            v(_imp->names.begin()), v_end(_imp->names.end()) ; v != v_end ; ++v)
    {
        tr1::shared_ptr<const PackageIDSequence> matches(_imp->env->package_database()->query(
                    query::Matches(*v->second) &
                    query::SupportsAction<InstallAction>(),
                    qo_order_by_version));

        if (matches->empty())
            Log::get_instance()->message(ll_warning, lc_context, "No packages matching '"
                    + stringify(*v->second) + "' for virtual '"
                    + stringify(v->first) + "'");

        for (PackageIDSequence::Iterator m(matches->begin()), m_end(matches->end()) ;
                m != m_end ; ++m)
        {
            IDMap::iterator i(_imp->ids.find(v->first));
            if (_imp->ids.end() == i)
                i = _imp->ids.insert(std::make_pair(v->first, make_shared_ptr(new PackageIDSequence))).first;

            tr1::shared_ptr<const PackageID> id(make_virtual_package_id(QualifiedPackageName(v->first), *m));
            if (stringify(id->name().category) != "virtual")
                throw InternalError("PALUDIS_HERE", "Got bad id '" + stringify(*id) + "'");
            i->second->push_back(id);
        }
    }

    _imp->has_ids = true;
}

tr1::shared_ptr<Repository>
VirtualsRepository::make_virtuals_repository(
        Environment * const env,
        tr1::shared_ptr<const Map<std::string, std::string> >)
{
    return tr1::shared_ptr<Repository>(new VirtualsRepository(env));
}

tr1::shared_ptr<const PackageIDSequence>
VirtualsRepository::do_package_ids(const QualifiedPackageName & q) const
{
    if (q.category.data() != "virtual")
        return tr1::shared_ptr<PackageIDSequence>(new PackageIDSequence);

    need_ids();

    IDMap::const_iterator i(_imp->ids.find(q));
    if (i == _imp->ids.end())
        return tr1::shared_ptr<PackageIDSequence>(new PackageIDSequence);

    return i->second;
}

tr1::shared_ptr<const QualifiedPackageNameSet>
VirtualsRepository::do_package_names(const CategoryNamePart & c) const
{
    if (c.data() != "virtual")
        return tr1::shared_ptr<QualifiedPackageNameSet>(new QualifiedPackageNameSet);

    need_ids();

    tr1::shared_ptr<QualifiedPackageNameSet> result(new QualifiedPackageNameSet);
    std::copy(_imp->ids.begin(), _imp->ids.end(), transform_inserter(result->inserter(),
                tr1::mem_fn(&std::pair<const QualifiedPackageName, tr1::shared_ptr<PackageIDSequence> >::first)));

    return result;
}

tr1::shared_ptr<const CategoryNamePartSet>
VirtualsRepository::do_category_names() const
{
    tr1::shared_ptr<CategoryNamePartSet> result(new CategoryNamePartSet);
    result->insert(CategoryNamePart("virtual"));
    return result;
}

bool
VirtualsRepository::do_has_package_named(const QualifiedPackageName & q) const
{
    if (q.category.data() != "virtual")
        return false;

    need_names();

    std::pair<
        std::vector<std::pair<QualifiedPackageName, tr1::shared_ptr<const PackageDepSpec> > >::const_iterator,
        std::vector<std::pair<QualifiedPackageName, tr1::shared_ptr<const PackageDepSpec> > >::const_iterator> p(
            std::equal_range(_imp->names.begin(), _imp->names.end(),
                std::make_pair(q, tr1::shared_ptr<const PackageDepSpec>()),
                NamesNameComparator()));

    return p.first != p.second;
}

bool
VirtualsRepository::do_has_category_named(const CategoryNamePart & c) const
{
    return (c.data() == "virtual");
}

void
VirtualsRepository::invalidate()
{
    _imp.reset(new Implementation<VirtualsRepository>(_imp->env));
}

void
VirtualsRepository::invalidate_masks()
{
    Lock l(_imp->big_nasty_mutex);

    for (IDMap::iterator it(_imp->ids.begin()), it_end(_imp->ids.end()); it_end != it; ++it)
        for (PackageIDSequence::Iterator it2(it->second->begin()), it2_end(it->second->end());
             it2_end != it2; ++it2)
            (*it2)->invalidate_masks();
}

const tr1::shared_ptr<const PackageID>
VirtualsRepository::make_virtual_package_id(
        const QualifiedPackageName & virtual_name, const tr1::shared_ptr<const PackageID> & provider) const
{
    if (virtual_name.category.data() != "virtual")
        throw InternalError(PALUDIS_HERE, "tried to make a virtual package id using '" + stringify(virtual_name) + "', '"
                + stringify(*provider) + "'");

    return make_shared_ptr(new virtuals::VirtualsPackageID(shared_from_this(), virtual_name, provider, true));
}

bool
VirtualsRepository::can_be_favourite_repository() const
{
    return false;
}

namespace
{
    struct SupportsActionQuery :
        ConstVisitor<SupportsActionTestVisitorTypes>
    {
        bool result;

        SupportsActionQuery() :
            result(false)
        {
        }

        void visit(const SupportsActionTest<InstalledAction> &)
        {
        }

        void visit(const SupportsActionTest<InstallAction> &)
        {
            result = true;
        }

        void visit(const SupportsActionTest<ConfigAction> &)
        {
        }

        void visit(const SupportsActionTest<PretendAction> &)
        {
        }

        void visit(const SupportsActionTest<FetchAction> &)
        {
        }

        void visit(const SupportsActionTest<UninstallAction> &)
        {
        }
    };
}

bool
VirtualsRepository::do_some_ids_might_support_action(const SupportsActionTestBase & a) const
{
    SupportsActionQuery q;
    a.accept(q);
    return q.result;
}

