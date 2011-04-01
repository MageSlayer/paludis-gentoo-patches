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
#include <paludis/util/stringify.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>

#include <istream>
#include <ostream>

using namespace paludis;

#include <paludis/package_dep_spec_constraint-se.cc>

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

InstalledAtPathConstraint::InstalledAtPathConstraint(const FSPath & n) :
    _path(n)
{
}

InstalledAtPathConstraint::~InstalledAtPathConstraint() = default;

const FSPath
InstalledAtPathConstraint::path() const
{
    return _path;
}

template class Pool<InstalledAtPathConstraint>;
template class Singleton<Pool<InstalledAtPathConstraint> >;
template const std::shared_ptr<const InstalledAtPathConstraint> Pool<InstalledAtPathConstraint>::create(
        const FSPath &) const;

InstallableToPathConstraint::InstallableToPathConstraint(const FSPath & n, const bool i) :
    _path(n),
    _include_masked(i)
{
}

InstallableToPathConstraint::~InstallableToPathConstraint() = default;

const FSPath
InstallableToPathConstraint::path() const
{
    return _path;
}

bool
InstallableToPathConstraint::include_masked() const
{
    return _include_masked;
}

template class Pool<InstallableToPathConstraint>;
template class Singleton<Pool<InstallableToPathConstraint> >;
template const std::shared_ptr<const InstallableToPathConstraint> Pool<InstallableToPathConstraint>::create(
        const FSPath &, const bool &) const;

InstallableToRepositoryConstraint::InstallableToRepositoryConstraint(const RepositoryName & n, const bool i) :
    _name(n),
    _include_masked(i)
{
}

InstallableToRepositoryConstraint::~InstallableToRepositoryConstraint() = default;

const RepositoryName
InstallableToRepositoryConstraint::name() const
{
    return _name;
}

bool
InstallableToRepositoryConstraint::include_masked() const
{
    return _include_masked;
}

template class Pool<InstallableToRepositoryConstraint>;
template class Singleton<Pool<InstallableToRepositoryConstraint> >;
template const std::shared_ptr<const InstallableToRepositoryConstraint> Pool<InstallableToRepositoryConstraint>::create(
        const RepositoryName &, const bool &) const;

ExactSlotConstraint::ExactSlotConstraint(const SlotName & n, const bool i) :
    _name(n),
    _locked(i)
{
}

ExactSlotConstraint::~ExactSlotConstraint() = default;

const SlotName
ExactSlotConstraint::name() const
{
    return _name;
}

bool
ExactSlotConstraint::locked() const
{
    return _locked;
}

template class Pool<ExactSlotConstraint>;
template class Singleton<Pool<ExactSlotConstraint> >;
template const std::shared_ptr<const ExactSlotConstraint> Pool<ExactSlotConstraint>::create(const SlotName &, const bool &) const;

AnySlotConstraint::AnySlotConstraint(const bool i) :
    _locking(i)
{
}

AnySlotConstraint::~AnySlotConstraint() = default;

bool
AnySlotConstraint::locking() const
{
    return _locking;
}

template class Pool<AnySlotConstraint>;
template class Singleton<Pool<AnySlotConstraint> >;
template const std::shared_ptr<const AnySlotConstraint> Pool<AnySlotConstraint>::create(const bool &) const;

KeyConstraint::KeyConstraint(const std::string & k, const KeyConstraintOperation o, const std::string & p) :
    _key(k),
    _operation(o),
    _pattern(p)
{
}

KeyConstraint::~KeyConstraint() = default;

const std::string
KeyConstraint::key() const
{
    return _key;
}

KeyConstraintOperation
KeyConstraint::operation() const
{
    return _operation;
}

const std::string
KeyConstraint::pattern() const
{
    return _pattern;
}

template class Pool<KeyConstraint>;
template class Singleton<Pool<KeyConstraint> >;
template const std::shared_ptr<const KeyConstraint> Pool<KeyConstraint>::create(const std::string &, const KeyConstraintOperation &, const std::string &) const;
template class Sequence<std::shared_ptr<const KeyConstraint> >;
template class WrappedForwardIterator<Sequence<std::shared_ptr<const KeyConstraint> >::ConstIteratorTag, const std::shared_ptr<const KeyConstraint> >;

