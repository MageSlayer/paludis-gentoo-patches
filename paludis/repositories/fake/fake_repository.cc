/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008 Ciaran McCreesh
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

#include <paludis/repositories/fake/fake_repository.hh>
#include <paludis/repositories/fake/fake_package_id.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/kc.hh>
#include <paludis/distribution.hh>
#include <paludis/environment.hh>
#include <paludis/package_id.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/action.hh>

#include <map>

using namespace paludis;

#include <paludis/repositories/fake/fake_repository-sr.cc>

namespace paludis
{
    template<>
    struct Implementation<FakeRepository>
    {
        std::tr1::shared_ptr<FakeRepository::VirtualsSequence> virtual_packages;
        std::map<std::string, std::string> mirrors;

        std::tr1::shared_ptr<const MetadataValueKey<std::string> > format_key;

        Implementation() :
            virtual_packages(new FakeRepository::VirtualsSequence),
            format_key(new LiteralMetadataValueKey<std::string> (
                        "format", "format", mkt_significant, "fake"))
        {
            mirrors.insert(std::make_pair("example", "http://fake-example/fake-example/"));
            mirrors.insert(std::make_pair("repo", "http://fake-repo/fake-repo/"));
        }
    };
}

FakeRepository::FakeRepository(const Environment * const env, const RepositoryName & r) :
    PrivateImplementationPattern<FakeRepository>(new Implementation<FakeRepository>),
    FakeRepositoryBase(env, r, RepositoryCapabilities::named_create()
            (k::sets_interface(), this)
            (k::syncable_interface(), static_cast<RepositorySyncableInterface *>(0))
            (k::use_interface(), this)
            (k::mirrors_interface(), this)
            (k::environment_variable_interface(), static_cast<RepositoryEnvironmentVariableInterface *>(0))
            (k::provides_interface(), static_cast<RepositoryProvidesInterface *>(0))
            (k::virtuals_interface(), (*DistributionData::get_instance()->distribution_from_string(
                    env->default_distribution()))[k::support_old_style_virtuals()] ? this : 0)
            (k::destination_interface(), static_cast<RepositoryDestinationInterface *>(0))
            (k::e_interface(), static_cast<RepositoryEInterface *>(0))
            (k::make_virtuals_interface(), static_cast<RepositoryMakeVirtualsInterface *>(0))
            (k::qa_interface(), static_cast<RepositoryQAInterface *>(0))
            (k::hook_interface(), static_cast<RepositoryHookInterface *>(0))
            (k::manifest_interface(), static_cast<RepositoryManifestInterface *>(0))),
            _imp(PrivateImplementationPattern<FakeRepository>::_imp)
{
    add_metadata_key(_imp->format_key);
}

FakeRepository::FakeRepository(const FakeRepositoryParams & params) :
    PrivateImplementationPattern<FakeRepository>(new Implementation<FakeRepository>),
    FakeRepositoryBase(params.environment, params.name, RepositoryCapabilities::named_create()
            (k::sets_interface(), this)
            (k::syncable_interface(), static_cast<RepositorySyncableInterface *>(0))
            (k::use_interface(), this)
            (k::mirrors_interface(), this)
            (k::environment_variable_interface(), static_cast<RepositoryEnvironmentVariableInterface *>(0))
            (k::provides_interface(), static_cast<RepositoryProvidesInterface *>(0))
            (k::virtuals_interface(), (*DistributionData::get_instance()->distribution_from_string(
                    params.environment->default_distribution()))[k::support_old_style_virtuals()] ? this : 0)
            (k::destination_interface(), static_cast<RepositoryDestinationInterface *>(0))
            (k::e_interface(), static_cast<RepositoryEInterface *>(0))
            (k::make_virtuals_interface(), static_cast<RepositoryMakeVirtualsInterface *>(0))
            (k::qa_interface(), static_cast<RepositoryQAInterface *>(0))
            (k::hook_interface(), static_cast<RepositoryHookInterface *>(0))
            (k::manifest_interface(), static_cast<RepositoryManifestInterface *>(0))),
            _imp(PrivateImplementationPattern<FakeRepository>::_imp)
{
    add_metadata_key(_imp->format_key);
}

FakeRepository::~FakeRepository()
{
}

std::tr1::shared_ptr<const FakeRepository::VirtualsSequence>
FakeRepository::virtual_packages() const
{
    return _imp->virtual_packages;
}

void
FakeRepository::add_virtual_package(const QualifiedPackageName & q, std::tr1::shared_ptr<const PackageDepSpec> p)
{
    _imp->virtual_packages->push_back(RepositoryVirtualsEntry(q, p));
}

namespace paludis
{
    class RepositoryMaker;
}

extern "C"
{
    void PALUDIS_VISIBLE register_repositories(RepositoryMaker * maker);
}

void register_repositories(RepositoryMaker *)
{
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

        void visit(const SupportsActionTest<PretendFetchAction> &)
        {
        }

        void visit(const SupportsActionTest<FetchAction> &)
        {
        }

        void visit(const SupportsActionTest<InfoAction> &)
        {
        }

        void visit(const SupportsActionTest<UninstallAction> &)
        {
        }
    };
}

bool
FakeRepository::some_ids_might_support_action(const SupportsActionTestBase & a) const
{
    SupportsActionQuery q;
    a.accept(q);
    return q.result;
}

FakeRepository::MirrorsConstIterator
FakeRepository::begin_mirrors(const std::string & s) const
{
    return MirrorsConstIterator(_imp->mirrors.equal_range(s).first);
}

FakeRepository::MirrorsConstIterator
FakeRepository::end_mirrors(const std::string & s) const
{
    return MirrorsConstIterator(_imp->mirrors.equal_range(s).second);
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
FakeRepository::format_key() const
{
    return _imp->format_key;
}

const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >
FakeRepository::installed_root_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >();
}

