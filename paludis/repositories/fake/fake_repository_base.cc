/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010 Ciaran McCreesh
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
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/hook.hh>
#include <functional>
#include <map>
#include <algorithm>

/** \file
 * Imp for FakeRepositoryBase.
 *
 * \ingroup grpfakerepository
 */

using namespace paludis;

namespace paludis
{
    template<>
    struct Imp<FakeRepositoryBase>
    {
        std::shared_ptr<CategoryNamePartSet> category_names;
        std::map<CategoryNamePart, std::shared_ptr<PackageNamePartSet> > package_names;
        std::map<QualifiedPackageName, std::shared_ptr<PackageIDSequence> > ids;

        const Environment * const env;

        Imp(const Environment * const);
    };

    Imp<FakeRepositoryBase>::Imp(const Environment * const e) :
        category_names(std::make_shared<CategoryNamePartSet>()),
        env(e)
    {
    }
}

FakeRepositoryBase::FakeRepositoryBase(const Environment * const e,
        const RepositoryName & our_name, const RepositoryCapabilities & caps) :
    Repository(e, our_name, caps),
    Pimp<FakeRepositoryBase>(e),
    _imp(Pimp<FakeRepositoryBase>::_imp)
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

std::shared_ptr<const CategoryNamePartSet>
FakeRepositoryBase::category_names() const
{
    return _imp->category_names;
}

std::shared_ptr<const QualifiedPackageNameSet>
FakeRepositoryBase::package_names(const CategoryNamePart & c) const
{
    std::shared_ptr<QualifiedPackageNameSet> result(std::make_shared<QualifiedPackageNameSet>());
    if (! has_category_named(c))
        return result;

    PackageNamePartSet::ConstIterator p(_imp->package_names.find(c)->second->begin()),
        p_end(_imp->package_names.find(c)->second->end());
    for ( ; p != p_end ; ++p)
        result->insert(c + *p);
    return result;
}

std::shared_ptr<const PackageIDSequence>
FakeRepositoryBase::package_ids(const QualifiedPackageName & n) const
{
    if (! has_category_named(n.category()))
        return std::make_shared<PackageIDSequence>();
    if (! has_package_named(n))
        return std::make_shared<PackageIDSequence>();
    return _imp->ids.find(n)->second;
}

void
FakeRepositoryBase::add_category(const CategoryNamePart & c)
{
    _imp->category_names->insert(c);
    _imp->package_names.insert(std::make_pair(c, std::make_shared<PackageNamePartSet>()));
}

void
FakeRepositoryBase::add_package(const QualifiedPackageName & q)
{
    add_category(q.category());
    _imp->package_names.find(q.category())->second->insert(q.package());
    _imp->ids.insert(std::make_pair(q, std::make_shared<PackageIDSequence>()));
}

namespace
{
    struct VersionIs
    {
        const VersionSpec version;

        VersionIs(const VersionSpec & v) :
            version(v)
        {
        }

        bool operator() (const std::shared_ptr<const PackageID> & i) const
        {
            return i->version() == version;
        }
    };
}

std::shared_ptr<FakePackageID>
FakeRepositoryBase::add_version(const QualifiedPackageName & q, const VersionSpec & v)
{
    add_package(q);

    if (_imp->ids.find(q)->second->end() != std::find_if(_imp->ids.find(q)->second->begin(),
                _imp->ids.find(q)->second->end(), VersionIs(v)))
        throw InternalError(PALUDIS_HERE, "duplicate id added");

    std::shared_ptr<FakePackageID> id(std::make_shared<FakePackageID>(_imp->env, name(), q, v));
    _imp->ids.find(q)->second->push_back(id);
    return id;
}

std::shared_ptr<FakePackageID>
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
    for (std::map<QualifiedPackageName, std::shared_ptr<PackageIDSequence> >::iterator it(_imp->ids.begin()), it_end(_imp->ids.end());
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
FakeRepositoryBase::perform_hook(const Hook &, const std::shared_ptr<OutputManager> &)
{
    return make_named_values<HookResult>(n::max_exit_status() = 0, n::output() = "");
}

bool
FakeRepositoryBase::sync(
        const std::string &,
        const std::shared_ptr<OutputManager> &) const
{
    return false;
}

