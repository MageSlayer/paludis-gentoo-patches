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

#include "config.h"

#include <paludis/repositories/nothing/nothing_repository.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/portage_dep_parser.hh>
#include <paludis/syncer.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/log.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/system.hh>
#include <paludis/util/tokeniser.hh>

/** \file
 * Implementation of NothingRepository.
 *
 * \ingroup grpnothingrepository
 */

using namespace paludis;

#include <paludis/repositories/nothing/nothing_repository-sr.cc>

namespace paludis
{
    /**
     * Implementation data for a NothingRepository.
     *
     * \ingroup grpnothingrepository
     */
    template <>
    struct Implementation<NothingRepository> :
        InternalCounted<Implementation<NothingRepository> >
    {
        /// Our name
        std::string name;

        /// Our location
        FSEntry location;

        /// Sync URL
        std::string sync;

        /// Sync exclude file
        std::string sync_exclude;

        /// (Empty) provides map.
        const std::map<QualifiedPackageName, QualifiedPackageName> provide_map;

        /// Constructor.
        Implementation(const NothingRepositoryParams &);

        /// Destructor.
        ~Implementation();
    };

    Implementation<NothingRepository>::Implementation(const NothingRepositoryParams & p) :
        name(p.name),
        location(p.location),
        sync(p.sync),
        sync_exclude(p.sync_exclude)
    {
    }

    Implementation<NothingRepository>::~Implementation()
    {
    }
}

NothingRepository::NothingRepository(const NothingRepositoryParams & p) try :
    Repository(RepositoryName(p.name),
            RepositoryCapabilities::create()
            .mask_interface(0)
            .installable_interface(0)
            .installed_interface(0)
            .news_interface(0)
            .sets_interface(0)
            .syncable_interface(this)
            .uninstallable_interface(0)
            .use_interface(0)
            .world_interface(0)
            .environment_variable_interface(0)
            .mirrors_interface(0)
            .virtuals_interface(0)
            .provides_interface(0)),
    PrivateImplementationPattern<NothingRepository>(new Implementation<NothingRepository>(p))
{
    RepositoryInfoSection::Pointer config_info(new RepositoryInfoSection("Configuration information"));
    config_info->add_kv("sync", _imp->sync);
    config_info->add_kv("sync_exclude", _imp->sync_exclude);
    config_info->add_kv("location", stringify(_imp->location));

    _info->add_section(config_info);
}
catch (const NameError & e)
{
    Context context("When making Nothing repository '" + p.name + "':");
    throw NothingRepositoryConfigurationError("Caught NameError: " + e.message());
}

NothingRepository::~NothingRepository()
{
}

bool
NothingRepository::do_has_category_named(const CategoryNamePart &) const
{
    return false;
}

bool
NothingRepository::do_has_package_named(const QualifiedPackageName &) const
{
    return false;
}

CategoryNamePartCollection::ConstPointer
NothingRepository::do_category_names() const
{
    CategoryNamePartCollection::Pointer result(new CategoryNamePartCollection::Concrete);
    return result;
}

QualifiedPackageNameCollection::ConstPointer
NothingRepository::do_package_names(const CategoryNamePart &) const
{
    QualifiedPackageNameCollection::Pointer result(new QualifiedPackageNameCollection::Concrete);
    return result;
}

VersionSpecCollection::ConstPointer
NothingRepository::do_version_specs(const QualifiedPackageName &) const
{
    return VersionSpecCollection::Pointer(new VersionSpecCollection::Concrete);
}

bool
NothingRepository::do_has_version(const QualifiedPackageName &,
        const VersionSpec &) const
{
    return false;
}

VersionMetadata::ConstPointer
NothingRepository::do_version_metadata(
        const QualifiedPackageName & q, const VersionSpec & v) const
{
    Log::get_instance()->message(ll_warning, lc_context, "has_version failed for request for '" +
            stringify(q) + "-" + stringify(v) + "' in repository '" +
            stringify(name()) + "'");
    return VersionMetadata::ConstPointer(new VersionMetadata::Ebuild(
                PortageDepParser::parse_depend));
}

CountedPtr<Repository>
NothingRepository::make_nothing_repository(
        const Environment * const,
        const PackageDatabase * const,
        AssociativeCollection<std::string, std::string>::ConstPointer m)
{
    std::string repo_file(m->end() == m->find("repo_file") ? std::string("?") :
            m->find("repo_file")->second);

    Context context("When making Nothing repository from repo_file '" + repo_file + "':");

    std::string location;
    if (m->end() == m->find("location") || ((location = m->find("location")->second)).empty())
        throw NothingRepositoryConfigurationError("Key 'location' not specified or empty");

    std::string sync;
    if (m->end() == m->find("sync") || ((sync = m->find("sync")->second)).empty())
        ; // nothing

    std::string name;
    if (m->end() == m->find("name") || ((name = m->find("name")->second)).empty())
        throw NothingRepositoryConfigurationError("Key 'name' not specified or empty in '"
                + repo_file + "'");

    std::string sync_exclude;
    if (m->end() == m->find("sync_exclude") || ((sync_exclude = m->find("sync_exclude")->second)).empty())
        ; // nothing

    return CountedPtr<Repository>(new NothingRepository(NothingRepositoryParams::create()
                .name(name)
                .location(location)
                .sync(sync)
                .sync_exclude(sync_exclude)));
}

NothingRepositoryConfigurationError::NothingRepositoryConfigurationError(
        const std::string & msg) throw () :
    ConfigurationError("Nothing repository configuration error: " + msg)
{
}

bool
NothingRepository::do_is_licence(const std::string &) const
{
    return false;
}

bool
NothingRepository::do_sync() const
{
    Context context("When syncing repository '" + stringify(name()) + "':");

    if (_imp->sync.empty())
        return false;

    std::string::size_type p(_imp->sync.find("://")), q(_imp->sync.find(":"));
    if (std::string::npos == p)
        throw NoSuchSyncerError(_imp->sync);

    SyncOptions opts(_imp->sync_exclude);

    SyncerMaker::get_instance()->find_maker(_imp->sync.substr(0, std::min(p, q)))(
            stringify(_imp->location), _imp->sync.substr(q < p ? q + 1 : 0))->sync(opts);

    return true;
}

void
NothingRepository::invalidate() const
{
}

