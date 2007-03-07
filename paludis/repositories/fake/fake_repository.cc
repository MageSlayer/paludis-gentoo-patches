/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include "fake_repository.hh"
#include <paludis/util/collection_concrete.hh>
#include <paludis/portage_dep_parser.hh>

using namespace paludis;

FakeRepository::FakeRepository(const Environment * const e, const RepositoryName & our_name) :
    FakeRepositoryBase(e, our_name, RepositoryCapabilities::create()
            .installable_interface(this)
            .installed_interface(0)
            .contents_interface(0)
            .mask_interface(this)
            .news_interface(0)
            .sets_interface(this)
            .syncable_interface(0)
            .uninstallable_interface(0)
            .use_interface(this)
            .world_interface(0)
            .mirrors_interface(0)
            .environment_variable_interface(0)
            .provides_interface(0)
            .virtuals_interface(this)
            .config_interface(0)
            .destination_interface(0)
            .licenses_interface(0),
            "fake"),
    _virtual_packages(new VirtualsCollection::Concrete)
{
}

void
FakeRepository::do_install(const QualifiedPackageName &, const VersionSpec &,
        const InstallOptions &) const
{
}

std::tr1::shared_ptr<const FakeRepository::VirtualsCollection>
FakeRepository::virtual_packages() const
{
    return _virtual_packages;
}

std::tr1::shared_ptr<const VersionMetadata>
FakeRepository::virtual_package_version_metadata(
        const RepositoryVirtualsEntry & p, const VersionSpec & v) const
{
    Context context("When fetching virtual package version metadata for '" + stringify(*p.provided_by_spec)
            + "' version '" + stringify(v) + "':");

    if (! p.provided_by_spec->package_ptr())
        throw ConfigurationError("Virtual provider atom does not specify an unambiguous package");

    std::tr1::shared_ptr<const VersionMetadata> m(version_metadata(*p.provided_by_spec->package_ptr(), v));
    std::tr1::shared_ptr<FakeVirtualVersionMetadata> result(new FakeVirtualVersionMetadata(
                m->slot, PackageDatabaseEntry(*p.provided_by_spec->package_ptr(), v, name())));

    result->eapi = m->eapi;
    result->deps_interface->build_depend_string = "=" + stringify(*p.provided_by_spec->package_ptr())
        + "-" + stringify(v);
    result->deps_interface->run_depend_string = "=" + stringify(*p.provided_by_spec->package_ptr())
        + "-" + stringify(v);

    return result;
}

void
FakeRepository::add_virtual_package(const QualifiedPackageName & q, std::tr1::shared_ptr<const PackageDepSpec> p)
{
    _virtual_packages->insert(RepositoryVirtualsEntry(q, p));
}

#ifndef MONOLITHIC

namespace paludis
{
    class RepositoryMaker;
}

extern "C"
{
    void register_repositories(RepositoryMaker * maker);
}

void register_repositories(RepositoryMaker *)
{
}

#endif

