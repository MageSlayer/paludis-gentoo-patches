/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2011 Ciaran McCreesh
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

#include <paludis/package_dep_spec_constraint.hh>
#include <paludis/util/pool-impl.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/singleton-impl.hh>

using namespace paludis;

PackageDepSpecConstraint::~PackageDepSpecConstraint() = default;

NameConstraint::NameConstraint(const QualifiedPackageName & n) :
    _name(n)
{
}

NameConstraint::~NameConstraint() = default;

const QualifiedPackageName
NameConstraint::name() const
{
    return _name;
}

template class Pool<NameConstraint>;
template class Singleton<Pool<NameConstraint> >;
template const std::shared_ptr<const NameConstraint> Pool<NameConstraint>::create(
        const QualifiedPackageName &) const;

CategoryNamePartConstraint::CategoryNamePartConstraint(const CategoryNamePart & n) :
    _name_part(n)
{
}

CategoryNamePartConstraint::~CategoryNamePartConstraint() = default;

const CategoryNamePart
CategoryNamePartConstraint::name_part() const
{
    return _name_part;
}

template class Pool<CategoryNamePartConstraint>;
template class Singleton<Pool<CategoryNamePartConstraint> >;
template const std::shared_ptr<const CategoryNamePartConstraint> Pool<CategoryNamePartConstraint>::create(
        const CategoryNamePart &) const;

PackageNamePartConstraint::PackageNamePartConstraint(const PackageNamePart & n) :
    _name_part(n)
{
}

PackageNamePartConstraint::~PackageNamePartConstraint() = default;

const PackageNamePart
PackageNamePartConstraint::name_part() const
{
    return _name_part;
}

template class Pool<PackageNamePartConstraint>;
template class Singleton<Pool<PackageNamePartConstraint> >;
template const std::shared_ptr<const PackageNamePartConstraint> Pool<PackageNamePartConstraint>::create(
        const PackageNamePart &) const;

InRepositoryConstraint::InRepositoryConstraint(const RepositoryName & n) :
    _name(n)
{
}

InRepositoryConstraint::~InRepositoryConstraint() = default;

const RepositoryName
InRepositoryConstraint::name() const
{
    return _name;
}

template class Pool<InRepositoryConstraint>;
template class Singleton<Pool<InRepositoryConstraint> >;
template const std::shared_ptr<const InRepositoryConstraint> Pool<InRepositoryConstraint>::create(
        const RepositoryName &) const;

FromRepositoryConstraint::FromRepositoryConstraint(const RepositoryName & n) :
    _name(n)
{
}

FromRepositoryConstraint::~FromRepositoryConstraint() = default;

const RepositoryName
FromRepositoryConstraint::name() const
{
    return _name;
}

template class Pool<FromRepositoryConstraint>;
template class Singleton<Pool<FromRepositoryConstraint> >;
template const std::shared_ptr<const FromRepositoryConstraint> Pool<FromRepositoryConstraint>::create(
        const RepositoryName &) const;

