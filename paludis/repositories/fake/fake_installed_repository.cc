/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009 Ciaran McCreesh
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
#include <paludis/util/fs_entry.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/set.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/environment.hh>
#include <paludis/dep_spec_flattener.hh>
#include <paludis/dep_spec.hh>
#include <paludis/action.hh>
#include <paludis/literal_metadata_key.hh>

using namespace paludis;

namespace paludis
{
    template <>
    struct Implementation<FakeInstalledRepository>
    {
        std::tr1::shared_ptr<const MetadataValueKey<std::string> > format_key;
        std::tr1::shared_ptr<const MetadataValueKey<FSEntry> > installed_root_key;
        const bool supports_uninstall;
        const bool is_suitable_destination;

        Implementation(const bool s, const bool b) :
            format_key(new LiteralMetadataValueKey<std::string> (
                        "format", "format", mkt_significant, "installed_fake")),
            installed_root_key(new LiteralMetadataValueKey<FSEntry> (
                        "installed_root", "installed_root", mkt_normal, FSEntry("/"))),
            supports_uninstall(s),
            is_suitable_destination(b)
        {
        }
    };
}

FakeInstalledRepository::FakeInstalledRepository(const Environment * const e, const RepositoryName & our_name,
        const bool supports_uninstall, const bool i) :
    FakeRepositoryBase(e, our_name, make_named_values<RepositoryCapabilities>(
                value_for<n::destination_interface>(this),
                value_for<n::e_interface>(static_cast<RepositoryEInterface *>(0)),
                value_for<n::environment_variable_interface>(static_cast<RepositoryEnvironmentVariableInterface *>(0)),
                value_for<n::make_virtuals_interface>(static_cast<RepositoryMakeVirtualsInterface *>(0)),
                value_for<n::manifest_interface>(static_cast<RepositoryManifestInterface *>(0)),
                value_for<n::mirrors_interface>(static_cast<RepositoryMirrorsInterface *>(0)),
                value_for<n::provides_interface>(this),
                value_for<n::syncable_interface>(static_cast<RepositorySyncableInterface *>(0)),
                value_for<n::virtuals_interface>(static_cast<RepositoryVirtualsInterface *>(0))
                )),
    PrivateImplementationPattern<FakeInstalledRepository>(new Implementation<FakeInstalledRepository>(supports_uninstall, i)),
    _imp(PrivateImplementationPattern<FakeInstalledRepository>::_imp)
{
    add_metadata_key(_imp->format_key);
    add_metadata_key(_imp->installed_root_key);
}

FakeInstalledRepository::~FakeInstalledRepository()
{
}

bool
FakeInstalledRepository::is_suitable_destination_for(const PackageID &) const
{
    return _imp->is_suitable_destination;
}

std::tr1::shared_ptr<const FakeInstalledRepository::ProvidesSequence>
FakeInstalledRepository::provided_packages() const
{
    std::tr1::shared_ptr<ProvidesSequence> result(new ProvidesSequence);

    std::tr1::shared_ptr<const CategoryNamePartSet> cats(category_names());
    for (CategoryNamePartSet::ConstIterator c(cats->begin()), c_end(cats->end()) ;
            c != c_end ; ++c)
    {
        std::tr1::shared_ptr<const QualifiedPackageNameSet> pkgs(package_names(*c));
        for (QualifiedPackageNameSet::ConstIterator p(pkgs->begin()), p_end(pkgs->end()) ;
                p != p_end ; ++p)
        {
            std::tr1::shared_ptr<const PackageIDSequence> vers(package_ids(*p));
            for (PackageIDSequence::ConstIterator v(vers->begin()), v_end(vers->end()) ;
                    v != v_end ; ++v)
            {
                if (! (*v)->provide_key())
                    continue;

                DepSpecFlattener<ProvideSpecTree, PackageDepSpec> f(environment());
                (*v)->provide_key()->value()->root()->accept(f);

                for (DepSpecFlattener<ProvideSpecTree, PackageDepSpec>::ConstIterator q(f.begin()), q_end(f.end()) ; q != q_end ; ++q)
                    result->push_back(make_named_values<RepositoryProvidesEntry>(
                            value_for<n::provided_by>(*v),
                            value_for<n::virtual_name>(QualifiedPackageName((*q)->text()))
                            ));
            }
        }
    }

    return result;
}

bool
FakeInstalledRepository::is_default_destination() const
{
    return environment()->root() == installed_root_key()->value();
}

bool
FakeInstalledRepository::want_pre_post_phases() const
{
    return true;
}

void
FakeInstalledRepository::merge(const MergeParams &)
{
}

namespace
{
    struct SupportsActionQuery
    {
        const bool supports_uninstall;

        SupportsActionQuery(const bool s) :
            supports_uninstall(s)
        {
        }

        bool visit(const SupportsActionTest<InstallAction> &) const
        {
            return false;
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

        bool visit(const SupportsActionTest<PretendFetchAction> &) const
        {
            return false;
        }

        bool visit(const SupportsActionTest<InfoAction> &) const
        {
            return false;
        }

        bool visit(const SupportsActionTest<UninstallAction> &) const
        {
            return supports_uninstall;
        }
    };
}

bool
FakeInstalledRepository::some_ids_might_support_action(const SupportsActionTestBase & a) const
{
    SupportsActionQuery q(_imp->supports_uninstall);
    return a.accept_returning<bool>(q);
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
FakeInstalledRepository::format_key() const
{
    return _imp->format_key;
}

const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >
FakeInstalledRepository::location_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >();
}

const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >
FakeInstalledRepository::installed_root_key() const
{
    return _imp->installed_root_key;
}

RepositoryName FakeInstalledRepository::repository_factory_name(
        const Environment * const,
        const std::tr1::function<std::string (const std::string &)> & f)
{
    return RepositoryName(f("name"));
}

std::tr1::shared_ptr<Repository>
FakeInstalledRepository::repository_factory_create(
        Environment * const env,
        const std::tr1::function<std::string (const std::string &)> & f)
{
    Context context("When creating FakeInstalledRepository:");
    RepositoryName name(f("name"));

    return make_shared_ptr(new FakeInstalledRepository(env, name));
}

std::tr1::shared_ptr<const RepositoryNameSet>
FakeInstalledRepository::repository_factory_dependencies(
        const Environment * const,
        const std::tr1::function<std::string (const std::string&)> &)
{
    return make_shared_ptr(new RepositoryNameSet);
}

