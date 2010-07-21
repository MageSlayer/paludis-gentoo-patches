/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010 Ciaran McCreesh
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
#include <paludis/match_package.hh>
#include <paludis/package_database.hh>
#include <paludis/action.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/selection.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/hook.hh>

#include <paludis/util/log.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/operators.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/map.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/wrapped_output_iterator.hh>

#include <tr1/functional>
#include <tr1/unordered_map>
#include <vector>
#include <utility>
#include <algorithm>

using namespace paludis;

typedef std::tr1::unordered_map<QualifiedPackageName, std::tr1::shared_ptr<PackageIDSequence>, Hash<QualifiedPackageName> > IDMap;

namespace paludis
{
    template<>
    struct Implementation<VirtualsRepository>
    {
        const Environment * const env;

        const std::tr1::shared_ptr<Mutex> big_nasty_mutex;

        mutable std::vector<std::pair<QualifiedPackageName, std::tr1::shared_ptr<const PackageDepSpec> > > names;
        mutable bool has_names;

        mutable IDMap ids;
        mutable bool has_ids;

        std::tr1::shared_ptr<const MetadataValueKey<std::string> > format_key;

        Implementation(const Environment * const e, std::tr1::shared_ptr<Mutex> m = make_shared_ptr(new Mutex)) :
            env(e),
            big_nasty_mutex(m),
            has_names(false),
            has_ids(false),
            format_key(new LiteralMetadataValueKey<std::string> (
                        "format", "format", mkt_significant, "virtuals"))
        {
        }
    };
}

namespace
{
    struct NamesNameComparator
    {
        bool
        operator() (const std::pair<QualifiedPackageName, std::tr1::shared_ptr<const PackageDepSpec> > & a,
                const std::pair<QualifiedPackageName, std::tr1::shared_ptr<const PackageDepSpec> > & b) const
        {
            return a.first < b.first;
        }
    };

    struct NamesSortComparator
    {
        bool
        operator() (const std::pair<QualifiedPackageName, std::tr1::shared_ptr<const PackageDepSpec> > & a,
                const std::pair<QualifiedPackageName, std::tr1::shared_ptr<const PackageDepSpec> > & b) const
        {
            if (a.first < b.first)
                return true;
            if (a.first > b.first)
                return false;
            return stringify(*a.second) < stringify(*b.second);
        }
    };

    struct NamesUniqueComparator
    {
        bool
        operator() (const std::pair<QualifiedPackageName, std::tr1::shared_ptr<const PackageDepSpec> > & a,
                const std::pair<QualifiedPackageName, std::tr1::shared_ptr<const PackageDepSpec> > & b) const
        {
            return a.first == b.first && stringify(*a.second) == stringify(*b.second);
        }
    };
}

VirtualsRepository::VirtualsRepository(const Environment * const env) :
    Repository(env, RepositoryName("virtuals"), make_named_values<RepositoryCapabilities>(
                n::destination_interface() = static_cast<RepositoryDestinationInterface *>(0),
                n::environment_variable_interface() = static_cast<RepositoryEnvironmentVariableInterface *>(0),
                n::make_virtuals_interface() = this,
                n::manifest_interface() = static_cast<RepositoryManifestInterface *>(0),
                n::provides_interface() = static_cast<RepositoryProvidesInterface *>(0),
                n::virtuals_interface() = static_cast<RepositoryVirtualsInterface *>(0)
            )),
    PrivateImplementationPattern<VirtualsRepository>(
            new Implementation<VirtualsRepository>(env)),
    _imp(PrivateImplementationPattern<VirtualsRepository>::_imp)
{
    add_metadata_key(_imp->format_key);
}

VirtualsRepository::~VirtualsRepository()
{
}

void
VirtualsRepository::need_names() const
{
    Lock l(*_imp->big_nasty_mutex);

    if (_imp->has_names)
        return;

    Context context("When loading names for virtuals repository:");

    Log::get_instance()->message("virtuals.need_names", ll_debug, lc_context) << "VirtualsRepository need_names";

    /* Determine our virtual name -> package mappings. */
    for (PackageDatabase::RepositoryConstIterator r(_imp->env->package_database()->begin_repositories()),
            r_end(_imp->env->package_database()->end_repositories()) ; r != r_end ; ++r)
    {
        if (! (**r).provides_interface())
            continue;

        std::tr1::shared_ptr<const RepositoryProvidesInterface::ProvidesSequence> provides(
                (**r).provides_interface()->provided_packages());
        for (RepositoryProvidesInterface::ProvidesSequence::ConstIterator p(provides->begin()),
                p_end(provides->end()) ; p != p_end ; ++p)
            _imp->names.push_back(std::make_pair((*p).virtual_name(), std::tr1::shared_ptr<const PackageDepSpec>(
                            new PackageDepSpec(make_package_dep_spec(PartiallyMadePackageDepSpecOptions()).package((*p).provided_by()->name())))));
    }

    std::sort(_imp->names.begin(), _imp->names.end(), NamesSortComparator());
    _imp->names.erase(std::unique(_imp->names.begin(), _imp->names.end(), NamesUniqueComparator()), _imp->names.end());

    std::vector<std::pair<QualifiedPackageName, std::tr1::shared_ptr<const PackageDepSpec> > > new_names;

    for (PackageDatabase::RepositoryConstIterator r(_imp->env->package_database()->begin_repositories()),
            r_end(_imp->env->package_database()->end_repositories()) ; r != r_end ; ++r)
    {
        if (! (**r).virtuals_interface())
            continue;

        std::tr1::shared_ptr<const RepositoryVirtualsInterface::VirtualsSequence> virtuals(
                (**r).virtuals_interface()->virtual_packages());
        for (RepositoryVirtualsInterface::VirtualsSequence::ConstIterator v(virtuals->begin()),
                v_end(virtuals->end()) ; v != v_end ; ++v)
        {
            std::pair<
                std::vector<std::pair<QualifiedPackageName, std::tr1::shared_ptr<const PackageDepSpec> > >::const_iterator,
                std::vector<std::pair<QualifiedPackageName, std::tr1::shared_ptr<const PackageDepSpec> > >::const_iterator> p(
                        std::equal_range(_imp->names.begin(), _imp->names.end(),
                            std::make_pair((*v).virtual_name(), std::tr1::shared_ptr<const PackageDepSpec>()),
                            NamesNameComparator()));

            if (p.first == p.second)
                new_names.push_back(std::make_pair((*v).virtual_name(), (*v).provided_by_spec()));
        }
    }

    std::copy(new_names.begin(), new_names.end(), std::back_inserter(_imp->names));
    std::sort(_imp->names.begin(), _imp->names.end(), NamesSortComparator());
    _imp->names.erase(std::unique(_imp->names.begin(), _imp->names.end(), NamesUniqueComparator()), _imp->names.end());

    _imp->has_names = true;
}

void
VirtualsRepository::need_ids() const
{
    Lock l(*_imp->big_nasty_mutex);

    if (_imp->has_ids)
        return;

    _imp->has_ids = true;

    Context context("When loading entries for virtuals repository:");
    need_names();

    Log::get_instance()->message("virtuals.need_entries", ll_debug, lc_context) << "VirtualsRepository need_entries";

    IDMap my_ids;

    /* Populate our _imp->entries. */
    for (std::vector<std::pair<QualifiedPackageName, std::tr1::shared_ptr<const PackageDepSpec> > >::const_iterator
            v(_imp->names.begin()), v_end(_imp->names.end()) ; v != v_end ; ++v)
    {
        std::tr1::shared_ptr<const PackageIDSequence> matches((*_imp->env)[selection::AllVersionsSorted(
                    generator::Matches(*v->second, MatchPackageOptions()) |
                    filter::SupportsAction<InstallAction>())]);

        if (matches->empty())
            Log::get_instance()->message("virtuals.no_match", ll_warning, lc_context) << "No packages matching '"
                    << *v->second << "' for virtual '" << v->first << "'";

        for (PackageIDSequence::ConstIterator m(matches->begin()), m_end(matches->end()) ;
                m != m_end ; ++m)
        {
            IDMap::iterator i(my_ids.find(v->first));
            if (my_ids.end() == i)
                i = my_ids.insert(std::make_pair(v->first, make_shared_ptr(new PackageIDSequence))).first;

            std::tr1::shared_ptr<const PackageID> id(make_virtual_package_id(QualifiedPackageName(v->first), *m));
            if (stringify(id->name().category()) != "virtual")
                throw InternalError(PALUDIS_HERE, "Got bad id '" + stringify(*id) + "'");
            i->second->push_back(id);
        }
    }

    using std::swap;
    swap(my_ids, _imp->ids);
}

std::tr1::shared_ptr<const PackageIDSequence>
VirtualsRepository::package_ids(const QualifiedPackageName & q) const
{
    if (q.category().value() != "virtual")
        return std::tr1::shared_ptr<PackageIDSequence>(new PackageIDSequence);

    need_ids();

    IDMap::const_iterator i(_imp->ids.find(q));
    if (i == _imp->ids.end())
        return std::tr1::shared_ptr<PackageIDSequence>(new PackageIDSequence);

    return i->second;
}

std::tr1::shared_ptr<const QualifiedPackageNameSet>
VirtualsRepository::package_names(const CategoryNamePart & c) const
{
    if (c.value() != "virtual")
        return std::tr1::shared_ptr<QualifiedPackageNameSet>(new QualifiedPackageNameSet);

    need_ids();

    std::tr1::shared_ptr<QualifiedPackageNameSet> result(new QualifiedPackageNameSet);
    std::transform(_imp->ids.begin(), _imp->ids.end(), result->inserter(),
            std::tr1::mem_fn(&std::pair<const QualifiedPackageName, std::tr1::shared_ptr<PackageIDSequence> >::first));

    return result;
}

std::tr1::shared_ptr<const CategoryNamePartSet>
VirtualsRepository::category_names() const
{
    std::tr1::shared_ptr<CategoryNamePartSet> result(new CategoryNamePartSet);
    result->insert(CategoryNamePart("virtual"));
    return result;
}

bool
VirtualsRepository::has_package_named(const QualifiedPackageName & q) const
{
    if (q.category().value() != "virtual")
        return false;

    need_names();

    std::pair<
        std::vector<std::pair<QualifiedPackageName, std::tr1::shared_ptr<const PackageDepSpec> > >::const_iterator,
        std::vector<std::pair<QualifiedPackageName, std::tr1::shared_ptr<const PackageDepSpec> > >::const_iterator> p(
            std::equal_range(_imp->names.begin(), _imp->names.end(),
                std::make_pair(q, std::tr1::shared_ptr<const PackageDepSpec>()),
                NamesNameComparator()));

    return p.first != p.second;
}

bool
VirtualsRepository::has_category_named(const CategoryNamePart & c) const
{
    return (c.value() == "virtual");
}

void
VirtualsRepository::invalidate()
{
    Lock l(*_imp->big_nasty_mutex);
    _imp.reset(new Implementation<VirtualsRepository>(_imp->env, _imp->big_nasty_mutex));
}

void
VirtualsRepository::invalidate_masks()
{
    Lock l(*_imp->big_nasty_mutex);

    for (IDMap::iterator it(_imp->ids.begin()), it_end(_imp->ids.end()); it_end != it; ++it)
        for (PackageIDSequence::ConstIterator it2(it->second->begin()), it2_end(it->second->end());
             it2_end != it2; ++it2)
            (*it2)->invalidate_masks();
}

const std::tr1::shared_ptr<const PackageID>
VirtualsRepository::make_virtual_package_id(
        const QualifiedPackageName & virtual_name, const std::tr1::shared_ptr<const PackageID> & provider) const
{
    if (virtual_name.category().value() != "virtual")
        throw InternalError(PALUDIS_HERE, "tried to make a virtual package id using '" + stringify(virtual_name) + "', '"
                + stringify(*provider) + "'");

    return make_shared_ptr(new virtuals::VirtualsPackageID(_imp->env, shared_from_this(), virtual_name, provider, true));
}

bool
VirtualsRepository::can_be_favourite_repository() const
{
    return false;
}

const bool
VirtualsRepository::is_unimportant() const
{
    return false;
}

namespace
{
    struct SupportsActionQuery
    {
        bool visit(const SupportsActionTest<InstallAction> &) const
        {
            return true;
        }

        bool visit(const SupportsActionTest<ConfigAction> &) const
        {
            return false;
        }

        bool visit(const SupportsActionTest<PretendAction> &) const
        {
            return false;
        }

        bool visit(const SupportsActionTest<FetchAction> &) const
        {
            return false;
        }

        bool visit(const SupportsActionTest<InfoAction> &) const
        {
            return false;
        }

        bool visit(const SupportsActionTest<UninstallAction> &) const
        {
            return false;
        }

        bool visit(const SupportsActionTest<PretendFetchAction> &) const
        {
            return false;
        }
    };
}

bool
VirtualsRepository::some_ids_might_support_action(const SupportsActionTestBase & a) const
{
    SupportsActionQuery q;
    return a.accept_returning<bool>(q);
}

bool
VirtualsRepository::some_ids_might_not_be_masked() const
{
    return true;
}

std::tr1::shared_ptr<const CategoryNamePartSet>
VirtualsRepository::unimportant_category_names() const
{
    std::tr1::shared_ptr<CategoryNamePartSet> result(make_shared_ptr(new CategoryNamePartSet));
    result->insert(CategoryNamePart("virtual"));
    return result;
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
VirtualsRepository::format_key() const
{
    return _imp->format_key;
}

const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >
VirtualsRepository::location_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >();
}

const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >
VirtualsRepository::installed_root_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >();
}

void
VirtualsRepository::need_keys_added() const
{
}

RepositoryName
VirtualsRepository::repository_factory_name(
        const Environment * const,
        const std::tr1::function<std::string (const std::string &)> &)
{
    return RepositoryName("virtuals");
}

std::tr1::shared_ptr<Repository>
VirtualsRepository::repository_factory_create(
        const Environment * const env,
        const std::tr1::function<std::string (const std::string &)> &)
{
    return make_shared_ptr(new VirtualsRepository(env));
}

std::tr1::shared_ptr<const RepositoryNameSet>
VirtualsRepository::repository_factory_dependencies(
        const Environment * const,
        const std::tr1::function<std::string (const std::string &)> &)
{
    return make_shared_ptr(new RepositoryNameSet);
}

void
VirtualsRepository::populate_sets() const
{
}

HookResult
VirtualsRepository::perform_hook(const Hook &)
{
    return make_named_values<HookResult>(n::max_exit_status() = 0, n::output() = "");
}

bool
VirtualsRepository::sync(const std::tr1::shared_ptr<OutputManager> &) const
{
    return false;
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
VirtualsRepository::accept_keywords_key() const
{
    return make_null_shared_ptr();
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
VirtualsRepository::sync_host_key() const
{
    return make_null_shared_ptr();
}

