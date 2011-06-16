/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#include <paludis/repositories/virtuals/installed_virtuals_repository.hh>

#include <paludis/environment.hh>
#include <paludis/hook.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/action.hh>

#include <paludis/util/pimp-impl.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/set.hh>
#include <paludis/util/map.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/log.hh>

#include <functional>
#include <unordered_map>
#include <algorithm>
#include <vector>

using namespace paludis;

namespace paludis
{
    template<>
    struct Imp<InstalledVirtualsRepository>
    {
        std::shared_ptr<const MetadataValueKey<FSPath> > root_key;
        std::shared_ptr<const MetadataValueKey<std::string> > format_key;

        Imp(const FSPath & r) :
            root_key(std::make_shared<LiteralMetadataValueKey<FSPath> >("root", "root", mkt_normal, r)),
            format_key(std::make_shared<LiteralMetadataValueKey<std::string> >("format", "format", mkt_significant, "installed_virtuals"))
        {
        }
    };
}

namespace
{
    struct MakeSafe
    {
        char operator() (const char & c) const
        {
            static const std::string allow(
                    "abcdefghijklmnopqrstuvwxyz"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "0123456789_-");

            if (std::string::npos == allow.find(c))
                return '-';
            else
                return c;
        }
    };

    RepositoryName
    make_name(const FSPath & r)
    {
        if (FSPath("/") == r)
            return RepositoryName("installed-virtuals");
        else
        {
            std::string n("installed-virtuals-" + stringify(r)), result;
            std::transform(n.begin(), n.end(), std::back_inserter(result), MakeSafe());
            return RepositoryName(result);
        }
    }
}

InstalledVirtualsRepository::InstalledVirtualsRepository(const Environment * const env,
        const FSPath & r) :
    Repository(env, RepositoryName(make_name(r)), make_named_values<RepositoryCapabilities>(
                n::destination_interface() = static_cast<RepositoryDestinationInterface *>(this),
                n::environment_variable_interface() = static_cast<RepositoryEnvironmentVariableInterface *>(0),
                n::manifest_interface() = static_cast<RepositoryManifestInterface *>(0)
            )),
    _imp(r)
{
    add_metadata_key(_imp->root_key);
    add_metadata_key(_imp->format_key);
}

InstalledVirtualsRepository::~InstalledVirtualsRepository()
{
}

std::shared_ptr<const PackageIDSequence>
InstalledVirtualsRepository::package_ids(const QualifiedPackageName &, const RepositoryContentMayExcludes &) const
{
    return std::make_shared<PackageIDSequence>();
}

std::shared_ptr<const QualifiedPackageNameSet>
InstalledVirtualsRepository::package_names(const CategoryNamePart &, const RepositoryContentMayExcludes &) const
{
    return std::make_shared<QualifiedPackageNameSet>();
}

std::shared_ptr<const CategoryNamePartSet>
InstalledVirtualsRepository::category_names(const RepositoryContentMayExcludes &) const
{
    std::shared_ptr<CategoryNamePartSet> result(std::make_shared<CategoryNamePartSet>());
    return result;
}

bool
InstalledVirtualsRepository::has_package_named(const QualifiedPackageName &, const RepositoryContentMayExcludes &) const
{
    return false;
}

bool
InstalledVirtualsRepository::has_category_named(const CategoryNamePart &, const RepositoryContentMayExcludes &) const
{
    return false;
}

void
InstalledVirtualsRepository::invalidate()
{
}

HookResult
InstalledVirtualsRepository::perform_hook(const Hook & hook, const std::shared_ptr<OutputManager> &)
{
    Context context("When performing hook '" + stringify(hook.name()) + "' for repository '"
            + stringify(name()) + "':");

    return make_named_values<HookResult>(n::max_exit_status() = 0, n::output() = "");
}

const bool
InstalledVirtualsRepository::is_unimportant() const
{
    return false;
}

bool
InstalledVirtualsRepository::some_ids_might_support_action(const SupportsActionTestBase &) const
{
    return false;
}

bool
InstalledVirtualsRepository::some_ids_might_not_be_masked() const
{
    return false;
}

std::shared_ptr<const CategoryNamePartSet>
InstalledVirtualsRepository::unimportant_category_names(const RepositoryContentMayExcludes &) const
{
    std::shared_ptr<CategoryNamePartSet> result(std::make_shared<CategoryNamePartSet>());
    return result;
}

const std::shared_ptr<const MetadataValueKey<std::string> >
InstalledVirtualsRepository::format_key() const
{
    return _imp->format_key;
}

const std::shared_ptr<const MetadataValueKey<FSPath> >
InstalledVirtualsRepository::location_key() const
{
    return std::shared_ptr<const MetadataValueKey<FSPath> >();
}

const std::shared_ptr<const MetadataValueKey<FSPath> >
InstalledVirtualsRepository::installed_root_key() const
{
    return _imp->root_key;
}

void
InstalledVirtualsRepository::need_keys_added() const
{
}

RepositoryName
InstalledVirtualsRepository::repository_factory_name(
        const Environment * const,
        const std::function<std::string (const std::string &)> & f)
{
    return make_name(FSPath(f("root")));
}

std::shared_ptr<Repository>
InstalledVirtualsRepository::repository_factory_create(
        const Environment * const env,
        const std::function<std::string (const std::string &)> & f)
{
    if (f("root").empty())
        throw ConfigurationError("Key 'root' unspecified or empty");

    Log::get_instance()->message("installed_virtuals.deprecated", ll_warning, lc_no_context)
        << "Old-style virtuals no longer exist. You should remove " << f("repo_file");

    return std::make_shared<InstalledVirtualsRepository>(env, FSPath(f("root")));
}

std::shared_ptr<const RepositoryNameSet>
InstalledVirtualsRepository::repository_factory_dependencies(
        const Environment * const,
        const std::function<std::string (const std::string &)> &)
{
    return std::make_shared<RepositoryNameSet>();
}

bool
InstalledVirtualsRepository::is_suitable_destination_for(const std::shared_ptr<const PackageID> &) const
{
    return false;
}

bool
InstalledVirtualsRepository::want_pre_post_phases() const
{
    return false;
}

void
InstalledVirtualsRepository::merge(const MergeParams &)
{
    throw InternalError(PALUDIS_HERE, "can't merge to installed virtuals");
}

void
InstalledVirtualsRepository::populate_sets() const
{
}

bool
InstalledVirtualsRepository::sync(
        const std::string &,
        const std::string &,
        const std::shared_ptr<OutputManager> &) const
{
    return false;
}

const std::shared_ptr<const MetadataCollectionKey<Map<std::string, std::string> > >
InstalledVirtualsRepository::sync_host_key() const
{
    return make_null_shared_ptr();
}

