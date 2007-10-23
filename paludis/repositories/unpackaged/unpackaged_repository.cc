/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#include <paludis/repositories/unpackaged/unpackaged_repository.hh>
#include <paludis/repositories/unpackaged/unpackaged_id.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/set.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/package_id.hh>
#include <paludis/action.hh>

using namespace paludis;
using namespace paludis::unpackaged_repositories;

#include <paludis/repositories/unpackaged/unpackaged_repository-sr.cc>

namespace paludis
{
    template <>
    struct Implementation<UnpackagedRepository>
    {
        const UnpackagedRepositoryParams params;
        tr1::shared_ptr<const PackageID> id;
        tr1::shared_ptr<PackageIDSequence> ids;
        tr1::shared_ptr<QualifiedPackageNameSet> package_names;
        tr1::shared_ptr<CategoryNamePartSet> category_names;

        Implementation(const RepositoryName & n,
                const UnpackagedRepositoryParams & p) :
            params(p),
            id(new UnpackagedID(params.environment, params.name, params.version, params.slot, n, params.location)),
            ids(new PackageIDSequence),
            package_names(new QualifiedPackageNameSet),
            category_names(new CategoryNamePartSet)
        {
            ids->push_back(id);
            package_names->insert(id->name());
            category_names->insert(id->name().category);
        }
    };
}

UnpackagedRepository::UnpackagedRepository(const RepositoryName & n,
        const UnpackagedRepositoryParams & params) :
    PrivateImplementationPattern<UnpackagedRepository>(new Implementation<UnpackagedRepository>(n, params)),
    Repository(n, RepositoryCapabilities::create()
            .installed_interface(0)
            .sets_interface(0)
            .syncable_interface(0)
            .use_interface(0)
            .world_interface(0)
            .mirrors_interface(0)
            .environment_variable_interface(0)
            .provides_interface(0)
            .virtuals_interface(0)
            .make_virtuals_interface(0)
            .destination_interface(0)
            .licenses_interface(0)
            .e_interface(0)
            .hook_interface(0)
            .qa_interface(0)
            .manifest_interface(0),
            "unpackaged")
{
}

UnpackagedRepository::~UnpackagedRepository()
{
}

tr1::shared_ptr<const PackageIDSequence>
UnpackagedRepository::do_package_ids(const QualifiedPackageName & n) const
{
    return n == _imp->id->name() ? _imp->ids : make_shared_ptr(new PackageIDSequence);
}

tr1::shared_ptr<const QualifiedPackageNameSet>
UnpackagedRepository::do_package_names(const CategoryNamePart & c) const
{
    return c == _imp->id->name().category ? _imp->package_names : make_shared_ptr(new QualifiedPackageNameSet);
}

tr1::shared_ptr<const CategoryNamePartSet>
UnpackagedRepository::do_category_names() const
{
    return _imp->category_names;
}

tr1::shared_ptr<const CategoryNamePartSet>
UnpackagedRepository::do_category_names_containing_package(const PackageNamePart & p) const
{
    return p == _imp->id->name().package ? _imp->category_names : make_shared_ptr(new CategoryNamePartSet);
}

bool
UnpackagedRepository::do_has_package_named(const QualifiedPackageName & q) const
{
    return q == _imp->id->name();
}

bool
UnpackagedRepository::do_has_category_named(const CategoryNamePart & c) const
{
    return c == _imp->id->name().category;
}

bool
UnpackagedRepository::do_some_ids_might_support_action(const SupportsActionTestBase & test) const
{
    return _imp->id->supports_action(test);
}

void
UnpackagedRepository::invalidate()
{
    _imp.reset(new Implementation<UnpackagedRepository>(name(), _imp->params));
}

void
UnpackagedRepository::invalidate_masks()
{
}

