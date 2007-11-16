/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh
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
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/set.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/visitor-impl.hh>
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
        tr1::shared_ptr<const MetadataStringKey> format_key;
        tr1::shared_ptr<const MetadataFSEntryKey> installed_root_key;

        Implementation() :
            format_key(new LiteralMetadataStringKey(
                        "format", "format", mkt_significant, "installed_fake")),
            installed_root_key(new LiteralMetadataFSEntryKey(
                        "installed_root", "installed_root", mkt_normal, FSEntry("/")))
        {
        }
    };
}

FakeInstalledRepository::FakeInstalledRepository(const Environment * const e, const RepositoryName & our_name) :
    FakeRepositoryBase(e, our_name, RepositoryCapabilities::create()
            .sets_interface(this)
            .syncable_interface(0)
            .use_interface(this)
            .world_interface(0)
            .mirrors_interface(0)
            .environment_variable_interface(0)
            .provides_interface(this)
            .virtuals_interface(0)
            .destination_interface(this)
            .e_interface(0)
            .make_virtuals_interface(0)
            .qa_interface(0)
            .hook_interface(0)
            .manifest_interface(0)),
    PrivateImplementationPattern<FakeInstalledRepository>(new Implementation<FakeInstalledRepository>),
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
    return true;
}

tr1::shared_ptr<const FakeInstalledRepository::ProvidesSequence>
FakeInstalledRepository::provided_packages() const
{
    tr1::shared_ptr<ProvidesSequence> result(new ProvidesSequence);

    tr1::shared_ptr<const CategoryNamePartSet> cats(category_names());
    for (CategoryNamePartSet::ConstIterator c(cats->begin()), c_end(cats->end()) ;
            c != c_end ; ++c)
    {
        tr1::shared_ptr<const QualifiedPackageNameSet> pkgs(package_names(*c));
        for (QualifiedPackageNameSet::ConstIterator p(pkgs->begin()), p_end(pkgs->end()) ;
                p != p_end ; ++p)
        {
            tr1::shared_ptr<const PackageIDSequence> vers(package_ids(*p));
            for (PackageIDSequence::ConstIterator v(vers->begin()), v_end(vers->end()) ;
                    v != v_end ; ++v)
            {
                if (! (*v)->provide_key())
                    continue;

                DepSpecFlattener<ProvideSpecTree, PackageDepSpec> f(environment(), **v);
                (*v)->provide_key()->value()->accept(f);

                for (DepSpecFlattener<ProvideSpecTree, PackageDepSpec>::ConstIterator q(f.begin()), q_end(f.end()) ; q != q_end ; ++q)
                    result->push_back(RepositoryProvidesEntry::create()
                            .virtual_name(QualifiedPackageName((*q)->text()))
                            .provided_by(*v));
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
    return false;
}

void
FakeInstalledRepository::merge(const MergeOptions &)
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
            result = true;
        }

        void visit(const SupportsActionTest<InstallAction> &)
        {
        }

        void visit(const SupportsActionTest<ConfigAction> &)
        {
        }

        void visit(const SupportsActionTest<PretendAction> &)
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
FakeInstalledRepository::some_ids_might_support_action(const SupportsActionTestBase & a) const
{
    SupportsActionQuery q;
    a.accept(q);
    return q.result;
}

const tr1::shared_ptr<const MetadataStringKey>
FakeInstalledRepository::format_key() const
{
    return _imp->format_key;
}

const tr1::shared_ptr<const MetadataFSEntryKey>
FakeInstalledRepository::installed_root_key() const
{
    return _imp->installed_root_key;
}

