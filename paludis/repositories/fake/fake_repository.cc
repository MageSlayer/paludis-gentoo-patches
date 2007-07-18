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

#include <paludis/repositories/fake/fake_repository.hh>
#include <paludis/repositories/fake/fake_package_id.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/distribution.hh>
#include <paludis/environment.hh>
#include <paludis/package_id.hh>
#include <paludis/action.hh>

#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <libwrapiter/libwrapiter_output_iterator.hh>

using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<FakeRepository>
    {
        tr1::shared_ptr<FakeRepository::VirtualsSequence> virtual_packages;

        Implementation() :
            virtual_packages(new FakeRepository::VirtualsSequence)
        {
        }
    };
}

FakeRepository::FakeRepository(const Environment * const e, const RepositoryName & our_name) :
    PrivateImplementationPattern<FakeRepository>(new Implementation<FakeRepository>),
    FakeRepositoryBase(e, our_name, RepositoryCapabilities::create()
            .installed_interface(0)
            .sets_interface(this)
            .syncable_interface(0)
            .use_interface(this)
            .world_interface(0)
            .mirrors_interface(0)
            .environment_variable_interface(0)
            .provides_interface(0)
            .virtuals_interface(DistributionData::get_instance()->distribution_from_string(
                    e->default_distribution())->support_old_style_virtuals ? this : 0)
            .destination_interface(0)
            .licenses_interface(0)
            .e_interface(0)
            .make_virtuals_interface(0)
            .qa_interface(0)
            .hook_interface(0),
            "fake"),
            _imp(PrivateImplementationPattern<FakeRepository>::_imp.get())
{
}

FakeRepository::~FakeRepository()
{
}

tr1::shared_ptr<const FakeRepository::VirtualsSequence>
FakeRepository::virtual_packages() const
{
    return _imp->virtual_packages;
}

void
FakeRepository::add_virtual_package(const QualifiedPackageName & q, tr1::shared_ptr<const PackageDepSpec> p)
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

        void visit(const SupportsActionTest<UninstallAction> &)
        {
        }
    };
}

bool
FakeRepository::do_some_ids_might_support_action(const SupportsActionTestBase & a) const
{
    SupportsActionQuery q;
    a.accept(q);
    return q.result;
}

