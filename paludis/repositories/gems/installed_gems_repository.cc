/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/repositories/gems/installed_gems_repository.hh>
#include <paludis/repositories/gems/params.hh>
#include <paludis/repositories/gems/metadata.hh>
#include <paludis/repositories/gems/gem_specification.hh>
#include <paludis/repositories/gems/yaml.hh>
#include <paludis/package_database.hh>
#include <paludis/environment.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/pstream.hh>
#include <paludis/util/system.hh>
#include <paludis/util/log.hh>
#include <paludis/util/strip.hh>
#include <paludis/hashed_containers.hh>
#include <paludis/eapi.hh>

using namespace paludis;

typedef MakeHashedMap<std::pair<QualifiedPackageName, VersionSpec>, tr1::shared_ptr<const gems::InstalledGemMetadata> >::Type MetadataMap;
typedef MakeHashedMap<QualifiedPackageName, tr1::shared_ptr<VersionSpecCollection> >::Type VersionsMap;

namespace paludis
{
    template <>
    struct Implementation<InstalledGemsRepository>
    {
        const gems::InstalledRepositoryParams params;

        mutable tr1::shared_ptr<const CategoryNamePartCollection> category_names;
        mutable MakeHashedMap<CategoryNamePart, tr1::shared_ptr<const QualifiedPackageNameCollection> >::Type package_names;
        mutable VersionsMap versions;
        mutable MetadataMap metadata;

        mutable bool has_category_names;
        mutable bool has_entries;

        Implementation(const gems::InstalledRepositoryParams p) :
            params(p),
            has_category_names(false),
            has_entries(false)
        {
        }
    };
}

InstalledGemsRepository::InstalledGemsRepository(const gems::InstalledRepositoryParams & params) :
    Repository(RepositoryName("installed-gems"),
            RepositoryCapabilities::create()
            .mask_interface(0)
            .installable_interface(0)
            .installed_interface(this)
            .sets_interface(0)
            .syncable_interface(0)
            .uninstallable_interface(this)
            .use_interface(0)
            .world_interface(0)
            .environment_variable_interface(0)
            .mirrors_interface(0)
            .virtuals_interface(0)
            .provides_interface(0)
            .contents_interface(0)
            .config_interface(0)
            .destination_interface(this)
            .licenses_interface(0)
            .portage_interface(0)
            .pretend_interface(0)
            .hook_interface(0),
            "installed_gems"),
    PrivateImplementationPattern<InstalledGemsRepository>(new Implementation<InstalledGemsRepository>(params))
{
    tr1::shared_ptr<RepositoryInfoSection> config_info(new RepositoryInfoSection("Configuration information"));

    config_info->add_kv("location", stringify(_imp->params.location));
    config_info->add_kv("buildroot", stringify(_imp->params.buildroot));

    _info->add_section(config_info);
}

InstalledGemsRepository::~InstalledGemsRepository()
{
}

void
InstalledGemsRepository::invalidate()
{
    _imp.reset(new Implementation<InstalledGemsRepository>(_imp->params));
}

bool
InstalledGemsRepository::do_has_category_named(const CategoryNamePart & c) const
{
    need_category_names();
    return _imp->category_names->end() != _imp->category_names->find(c);
}

bool
InstalledGemsRepository::do_has_package_named(const QualifiedPackageName & q) const
{
    if (! do_has_category_named(q.category))
        return false;

    need_entries();
    return _imp->package_names.find(q.category)->second->end() != _imp->package_names.find(q.category)->second->find(q);
}

tr1::shared_ptr<const CategoryNamePartCollection>
InstalledGemsRepository::do_category_names() const
{
    need_category_names();
    return _imp->category_names;
}

tr1::shared_ptr<const QualifiedPackageNameCollection>
InstalledGemsRepository::do_package_names(const CategoryNamePart & c) const
{
    if (! has_category_named(c))
        return make_shared_ptr(new QualifiedPackageNameCollection::Concrete);

    need_entries();

    MakeHashedMap<CategoryNamePart, tr1::shared_ptr<const QualifiedPackageNameCollection> >::Type::const_iterator i(
            _imp->package_names.find(c));
    if (i == _imp->package_names.end())
        return make_shared_ptr(new QualifiedPackageNameCollection::Concrete);
    return i->second;
}

tr1::shared_ptr<const VersionSpecCollection>
InstalledGemsRepository::do_version_specs(const QualifiedPackageName & q) const
{
    if (! has_package_named(q))
        return make_shared_ptr(new VersionSpecCollection::Concrete);

    need_entries();

    VersionsMap::const_iterator i(_imp->versions.find(q));
    if (i == _imp->versions.end())
        return make_shared_ptr(new VersionSpecCollection::Concrete);

    return i->second;
}

bool
InstalledGemsRepository::do_has_version(const QualifiedPackageName & q, const VersionSpec & v) const
{
    if (! has_package_named(q))
        return false;

    need_entries();

    VersionsMap::const_iterator i(_imp->versions.find(q));
    return i->second->end() != i->second->find(v);
}

tr1::shared_ptr<const VersionMetadata>
InstalledGemsRepository::do_version_metadata(const QualifiedPackageName & q, const VersionSpec & v) const
{
    if (! has_version(q, v))
        throw NoSuchPackageError(stringify(PackageDatabaseEntry(q, v, name())));

    need_version_metadata(q, v);
    return _imp->metadata.find(std::make_pair(q, v))->second;
}

void
InstalledGemsRepository::need_category_names() const
{
    if (_imp->has_category_names)
        return;

    tr1::shared_ptr<CategoryNamePartCollection::Concrete> cat(new CategoryNamePartCollection::Concrete);
    _imp->category_names = cat;

    cat->insert(CategoryNamePart("gems"));
    _imp->has_category_names = true;
}

void
InstalledGemsRepository::need_entries() const
{
    if (_imp->has_entries)
        return;

    static CategoryNamePart gems("gems");

    Context c("When loading entries for repository '" + stringify(name()) + "':");

    need_category_names();

    tr1::shared_ptr<QualifiedPackageNameCollection::Concrete> pkgs(new QualifiedPackageNameCollection::Concrete);
    _imp->package_names.insert(std::make_pair(gems, pkgs));

    for (DirIterator d(_imp->params.location / "specifications"), d_end ; d != d_end ; ++d)
    {
        if (! is_file_with_extension(*d, ".gemspec", IsFileWithOptions()))
            continue;

        std::string s(strip_trailing_string(d->basename(), ".gemspec"));
        std::string::size_type h(s.rfind('-'));
        if (std::string::npos == h)
        {
            Log::get_instance()->message(ll_qa, lc_context) << "Unrecognised file name format '"
                << *d << "' (no hyphen)";
            continue;
        }

        VersionSpec v(s.substr(h + 1));
        PackageNamePart p(s.substr(0, h));
        pkgs->insert(gems + p);

        if (_imp->versions.end() == _imp->versions.find(gems + p))
            _imp->versions.insert(std::make_pair(gems + p, make_shared_ptr(new VersionSpecCollection::Concrete)));
        _imp->versions.find(gems + p)->second->insert(v);
    }
}

void
InstalledGemsRepository::need_version_metadata(const QualifiedPackageName & q, const VersionSpec & v) const
{
    MetadataMap::const_iterator i(_imp->metadata.find(std::make_pair(q, v)));
    if (i != _imp->metadata.end())
        return;

    Context c("When loading version metadata for '" + stringify(PackageDatabaseEntry(q, v, name())) + "':");

    tr1::shared_ptr<gems::InstalledGemMetadata> m(new gems::InstalledGemMetadata(v));
    _imp->metadata.insert(std::make_pair(std::make_pair(q, v), m));

    Command cmd(getenv_with_default("PALUDIS_GEMS_DIR", LIBEXECDIR "/paludis") +
            "/gems/gems.bash specification '" + stringify(q.package) + "' '" + stringify(v) + "'");
    cmd.with_stderr_prefix(stringify(q) + "-" + stringify(v) + "::" + stringify(name()) + "> ");
    cmd.with_sandbox();
    cmd.with_uid_gid(_imp->params.environment->reduced_uid(), _imp->params.environment->reduced_gid());

    PStream p(cmd);
    std::string output((std::istreambuf_iterator<char>(p)), std::istreambuf_iterator<char>());

    if (0 != p.exit_status())
    {
        Log::get_instance()->message(ll_warning, lc_context) << "Version metadata extraction returned non-zero";
        return;
    }

    yaml::Document spec_doc(output);
    gems::GemSpecification spec(*spec_doc.top());
    m->populate_from_specification(spec);
    m->eapi = EAPIData::get_instance()->eapi_from_string("gems-1");
}

bool
InstalledGemsRepository::is_suitable_destination_for(const PackageDatabaseEntry & e) const
{
    std::string f(_imp->params.environment->package_database()->fetch_repository(e.repository)->format());
    return f == "gems";
}

bool
InstalledGemsRepository::is_default_destination() const
{
    return true;
}

bool
InstalledGemsRepository::want_pre_post_phases() const
{
    return true;
}

void
InstalledGemsRepository::merge(const MergeOptions &)
{
    throw InternalError(PALUDIS_HERE, "Invalid target for merge");
}

FSEntry
InstalledGemsRepository::root() const
{
    return FSEntry("/");
}

void
InstalledGemsRepository::do_uninstall(const QualifiedPackageName & q, const VersionSpec & v,
        const UninstallOptions &) const
{
    if (! has_version(q, v))
        throw NoSuchPackageError(stringify(PackageDatabaseEntry(q, v, name())));

    Command cmd(getenv_with_default("PALUDIS_GEMS_DIR", LIBEXECDIR "/paludis") +
            "/gems/gems.bash uninstall '" + stringify(q.package) + "' '" + stringify(v) + "'");
    cmd.with_stderr_prefix(stringify(q) + "-" + stringify(v) + "::" + stringify(name()) + "> ");
    cmd.with_setenv("GEMCACHE", stringify(_imp->params.location / "yaml"));

    if (0 != run_command(cmd))
        throw PackageInstallActionError("Uninstall of '" + stringify(PackageDatabaseEntry(q, v, name())) + "' failed");
}

