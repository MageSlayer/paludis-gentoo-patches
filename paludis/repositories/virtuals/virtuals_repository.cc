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

#include <paludis/repositories/virtuals/virtuals_repository.hh>

#include <paludis/environment.hh>
#include <paludis/match_package.hh>
#include <paludis/action.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/selection.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/hook.hh>
#include <paludis/partially_made_package_dep_spec.hh>

#include <paludis/util/log.hh>
#include <paludis/util/operators.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/map.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/make_null_shared_ptr.hh>

#include <functional>
#include <unordered_map>
#include <vector>
#include <utility>
#include <algorithm>

using namespace paludis;

typedef std::unordered_map<QualifiedPackageName, std::shared_ptr<PackageIDSequence>, Hash<QualifiedPackageName> > IDMap;

namespace paludis
{
    template<>
    struct Imp<VirtualsRepository>
    {
        std::shared_ptr<const MetadataValueKey<std::string> > format_key;

        Imp() :
            format_key(std::make_shared<LiteralMetadataValueKey<std::string> >("format", "format", mkt_significant, "virtuals"))
        {
        }
    };
}

VirtualsRepository::VirtualsRepository(const Environment * const env) :
    Repository(env, RepositoryName("virtuals"), make_named_values<RepositoryCapabilities>(
                n::destination_interface() = static_cast<RepositoryDestinationInterface *>(0),
                n::environment_variable_interface() = static_cast<RepositoryEnvironmentVariableInterface *>(0),
                n::manifest_interface() = static_cast<RepositoryManifestInterface *>(0)
            )),
    _imp()
{
    add_metadata_key(_imp->format_key);
}

VirtualsRepository::~VirtualsRepository()
{
}

std::shared_ptr<const PackageIDSequence>
VirtualsRepository::package_ids(const QualifiedPackageName &, const RepositoryContentMayExcludes &) const
{
    return std::make_shared<PackageIDSequence>();
}

std::shared_ptr<const QualifiedPackageNameSet>
VirtualsRepository::package_names(const CategoryNamePart &, const RepositoryContentMayExcludes &) const
{
    return std::make_shared<QualifiedPackageNameSet>();
}

std::shared_ptr<const CategoryNamePartSet>
VirtualsRepository::category_names(const RepositoryContentMayExcludes &) const
{
    std::shared_ptr<CategoryNamePartSet> result(std::make_shared<CategoryNamePartSet>());
    return result;
}

bool
VirtualsRepository::has_package_named(const QualifiedPackageName &, const RepositoryContentMayExcludes &) const
{
    return false;
}

bool
VirtualsRepository::has_category_named(const CategoryNamePart &, const RepositoryContentMayExcludes &) const
{
    return false;
}

void
VirtualsRepository::invalidate()
{
}

const bool
VirtualsRepository::is_unimportant() const
{
    return false;
}

bool
VirtualsRepository::some_ids_might_support_action(const SupportsActionTestBase &) const
{
    return false;
}

bool
VirtualsRepository::some_ids_might_not_be_masked() const
{
    return true;
}

std::shared_ptr<const CategoryNamePartSet>
VirtualsRepository::unimportant_category_names(const RepositoryContentMayExcludes &) const
{
    std::shared_ptr<CategoryNamePartSet> result(std::make_shared<CategoryNamePartSet>());
    return result;
}

const std::shared_ptr<const MetadataValueKey<std::string> >
VirtualsRepository::format_key() const
{
    return _imp->format_key;
}

const std::shared_ptr<const MetadataValueKey<FSPath> >
VirtualsRepository::location_key() const
{
    return std::shared_ptr<const MetadataValueKey<FSPath> >();
}

const std::shared_ptr<const MetadataValueKey<FSPath> >
VirtualsRepository::installed_root_key() const
{
    return std::shared_ptr<const MetadataValueKey<FSPath> >();
}

void
VirtualsRepository::need_keys_added() const
{
}

RepositoryName
VirtualsRepository::repository_factory_name(
        const Environment * const,
        const std::function<std::string (const std::string &)> &)
{
    return RepositoryName("virtuals");
}

std::shared_ptr<Repository>
VirtualsRepository::repository_factory_create(
        const Environment * const env,
        const std::function<std::string (const std::string &)> & f)
{
    Log::get_instance()->message("virtuals.deprecated", ll_warning, lc_no_context)
        << "Old-style virtuals no longer exist. You should remove " << f("repo_file");

    return std::make_shared<VirtualsRepository>(env);
}

std::shared_ptr<const RepositoryNameSet>
VirtualsRepository::repository_factory_dependencies(
        const Environment * const,
        const std::function<std::string (const std::string &)> &)
{
    return std::make_shared<RepositoryNameSet>();
}

void
VirtualsRepository::populate_sets() const
{
}

HookResult
VirtualsRepository::perform_hook(const Hook &, const std::shared_ptr<OutputManager> &)
{
    return make_named_values<HookResult>(n::max_exit_status() = 0, n::output() = "");
}

bool
VirtualsRepository::sync(
        const std::string &,
        const std::string &,
        const std::shared_ptr<OutputManager> &) const
{
    return false;
}

const std::shared_ptr<const MetadataCollectionKey<Map<std::string, std::string> > >
VirtualsRepository::sync_host_key() const
{
    return make_null_shared_ptr();
}

const std::shared_ptr<const Set<std::string> >
VirtualsRepository::maybe_expand_licence_nonrecursively(const std::string &) const
{
    return make_null_shared_ptr();
}

