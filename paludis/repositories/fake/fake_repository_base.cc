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

#include <paludis/repositories/fake/fake_repository_base.hh>
#include <paludis/repositories/fake/fake_package_id.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/hook.hh>
#include <tr1/functional>
#include <map>
#include <algorithm>

/** \file
 * Implementation for FakeRepositoryBase.
 *
 * \ingroup grpfakerepository
 */

using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<FakeRepositoryBase> :
        private InstantiationPolicy<Implementation<FakeRepositoryBase>, instantiation_method::NonCopyableTag>
    {
        std::tr1::shared_ptr<CategoryNamePartSet> category_names;
        std::map<CategoryNamePart, std::tr1::shared_ptr<PackageNamePartSet> > package_names;
        std::map<QualifiedPackageName, std::tr1::shared_ptr<PackageIDSequence> > ids;

        const Environment * const env;

        Implementation(const Environment * const);
    };

    Implementation<FakeRepositoryBase>::Implementation(const Environment * const e) :
        category_names(new CategoryNamePartSet),
        env(e)
    {
    }
}

FakeRepositoryBase::FakeRepositoryBase(const Environment * const e,
        const RepositoryName & our_name, const RepositoryCapabilities & caps) :
    Repository(e, our_name, caps),
    PrivateImplementationPattern<FakeRepositoryBase>(new Implementation<FakeRepositoryBase>(e)),
    _imp(PrivateImplementationPattern<FakeRepositoryBase>::_imp)
{
}

FakeRepositoryBase::~FakeRepositoryBase()
{
}

bool
FakeRepositoryBase::has_category_named(const CategoryNamePart & c) const
{
    return (_imp->category_names->end() != _imp->category_names->find(c));
}

bool
FakeRepositoryBase::has_package_named(const QualifiedPackageName & q) const
{
    return has_category_named(q.category()) &&
        (_imp->package_names.find(q.category())->second->end() !=
         _imp->package_names.find(q.category())->second->find(q.package()));
}

std::tr1::shared_ptr<const CategoryNamePartSet>
FakeRepositoryBase::category_names() const
{
    return _imp->category_names;
}

std::tr1::shared_ptr<const QualifiedPackageNameSet>
FakeRepositoryBase::package_names(const CategoryNamePart & c) const
{
    std::tr1::shared_ptr<QualifiedPackageNameSet> result(new QualifiedPackageNameSet);
    if (! has_category_named(c))
        return result;

    PackageNamePartSet::ConstIterator p(_imp->package_names.find(c)->second->begin()),
        p_end(_imp->package_names.find(c)->second->end());
    for ( ; p != p_end ; ++p)
        result->insert(c + *p);
    return result;
}

std::tr1::shared_ptr<const PackageIDSequence>
FakeRepositoryBase::package_ids(const QualifiedPackageName & n) const
{
    if (! has_category_named(n.category()))
        return std::tr1::shared_ptr<PackageIDSequence>(new PackageIDSequence);
    if (! has_package_named(n))
        return std::tr1::shared_ptr<PackageIDSequence>(new PackageIDSequence);
    return _imp->ids.find(n)->second;
}

void
FakeRepositoryBase::add_category(const CategoryNamePart & c)
{
    _imp->category_names->insert(c);
    _imp->package_names.insert(std::make_pair(c, new PackageNamePartSet));
}

void
FakeRepositoryBase::add_package(const QualifiedPackageName & q)
{
    add_category(q.category());
    _imp->package_names.find(q.category())->second->insert(q.package());
    _imp->ids.insert(std::make_pair(q, new PackageIDSequence));
}

std::tr1::shared_ptr<FakePackageID>
FakeRepositoryBase::add_version(const QualifiedPackageName & q, const VersionSpec & v)
{
    add_package(q);
    std::tr1::shared_ptr<FakePackageID> id(new FakePackageID(_imp->env, shared_from_this(), q, v));
    _imp->ids.find(q)->second->push_back(id);
    return id;
}

std::tr1::shared_ptr<FakePackageID>
FakeRepositoryBase::add_version(const std::string & c, const std::string & p,
        const std::string & v)
{
    return add_version(CategoryNamePart(c) + PackageNamePart(p), VersionSpec(v, user_version_spec_options()));
}

void
FakeRepositoryBase::invalidate()
{
}

void
FakeRepositoryBase::invalidate_masks()
{
    for (std::map<QualifiedPackageName, std::tr1::shared_ptr<PackageIDSequence> >::iterator it(_imp->ids.begin()), it_end(_imp->ids.end());
         it_end != it; ++it)
        for (PackageIDSequence::ConstIterator it2(it->second->begin()), it2_end(it->second->end());
             it2_end != it2; ++it2)
            (*it2)->invalidate_masks();
}

const Environment *
FakeRepositoryBase::environment() const
{
    return _imp->env;
}

void
FakeRepositoryBase::need_keys_added() const
{
}

void
FakeRepositoryBase::populate_sets() const
{
}

HookResult
FakeRepositoryBase::perform_hook(const Hook &)
{
    return make_named_values<HookResult>(value_for<n::max_exit_status>(0), value_for<n::output>(""));
}

