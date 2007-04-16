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

#include "fake_installed_repository.hh"
#include <paludis/util/collection_concrete.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/portage_dep_parser.hh>
#include <paludis/dep_spec_flattener.hh>

using namespace paludis;

FakeInstalledRepository::FakeInstalledRepository(const Environment * const e, const RepositoryName & our_name) :
    FakeRepositoryBase(e, our_name, RepositoryCapabilities::create()
            .installable_interface(0)
            .installed_interface(this)
            .contents_interface(0)
            .mask_interface(this)
            .sets_interface(this)
            .syncable_interface(0)
            .uninstallable_interface(0)
            .use_interface(this)
            .world_interface(0)
            .mirrors_interface(0)
            .environment_variable_interface(0)
            .provides_interface(this)
            .virtuals_interface(0)
            .config_interface(0)
            .destination_interface(this)
            .licenses_interface(0)
            .portage_interface(0)
            .hook_interface(0),
            "fake_installed")
{
}

bool
FakeInstalledRepository::is_suitable_destination_for(const PackageDatabaseEntry &) const
{
    return true;
}

std::tr1::shared_ptr<const FakeInstalledRepository::ProvidesCollection>
FakeInstalledRepository::provided_packages() const
{
    std::tr1::shared_ptr<ProvidesCollection> result(new ProvidesCollection::Concrete);

    std::tr1::shared_ptr<const CategoryNamePartCollection> cats(category_names());
    for (CategoryNamePartCollection::Iterator c(cats->begin()), c_end(cats->end()) ;
            c != c_end ; ++c)
    {
        std::tr1::shared_ptr<const QualifiedPackageNameCollection> pkgs(package_names(*c));
        for (QualifiedPackageNameCollection::Iterator p(pkgs->begin()), p_end(pkgs->end()) ;
                p != p_end ; ++p)
        {
            std::tr1::shared_ptr<const VersionSpecCollection> vers(version_specs(*p));
            for (VersionSpecCollection::Iterator v(vers->begin()), v_end(vers->end()) ;
                    v != v_end ; ++v)
            {
                std::tr1::shared_ptr<const VersionMetadata> m(version_metadata(*p, *v));
                if (! m->ebuild_interface)
                    continue;

                std::tr1::shared_ptr<const DepSpec> provide(m->ebuild_interface->provide());
                PackageDatabaseEntry dbe(*p, *v, name());
                DepSpecFlattener f(environment(), &dbe, provide);

                for (DepSpecFlattener::Iterator q(f.begin()), q_end(f.end()) ; q != q_end ; ++q)
                    result->insert(RepositoryProvidesEntry::create()
                            .virtual_name(QualifiedPackageName((*q)->text()))
                            .version(*v)
                            .provided_by_name(*p));
            }
        }
    }

    return result;
}

std::tr1::shared_ptr<const VersionMetadata>
FakeInstalledRepository::provided_package_version_metadata(const RepositoryProvidesEntry & p) const
{
    std::tr1::shared_ptr<const VersionMetadata> m(version_metadata(p.provided_by_name, p.version));
    std::tr1::shared_ptr<FakeVirtualVersionMetadata> result(new FakeVirtualVersionMetadata(
                m->slot, PackageDatabaseEntry(p.provided_by_name, p.version, name())));

    result->eapi = m->eapi;
    result->deps_interface->build_depend_string = stringify(p.provided_by_name);
    result->deps_interface->run_depend_string = stringify(p.provided_by_name);

    return result;
}

FSEntry
FakeInstalledRepository::root() const
{
    return FSEntry("/");
}

bool
FakeInstalledRepository::is_default_destination() const
{
    return environment()->root() == root();
}

bool
FakeInstalledRepository::want_pre_post_phases() const
{
    return false;
}

void
FakeInstalledRepository::merge(const MergeOptions &)
{
}

