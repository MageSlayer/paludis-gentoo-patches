/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#include <paludis/environment.hh>
#include <paludis/filter.hh>

#include <paludis/util/stringify.hh>
#include <paludis/util/join.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/sequence-impl.hh>

#include <list>
#include <algorithm>

using namespace paludis;

Environment::~Environment()
{
}

const Filter &
Environment::all_filter()
{
    static const Filter result((filter::All()));
    return result;
}

namespace paludis
{
    template <>
    struct WrappedForwardIteratorTraits<AmbiguousPackageNameError::OptionsConstIteratorTag>
    {
        typedef std::list<std::string>::const_iterator UnderlyingIterator;
    };
}

DuplicateRepositoryError::DuplicateRepositoryError(const std::string & name) noexcept :
    Exception("A repository named '" + name + "' already exists")
{
}

NoSuchPackageError::NoSuchPackageError(const std::string & our_name) noexcept :
    Exception("Could not find '" + our_name + "'"),
    _name(our_name)
{
}

NoSuchRepositoryError::NoSuchRepositoryError(const RepositoryName & n) noexcept :
    Exception("Could not find repository '" + stringify(n) + "'"),
    _name(n)
{
}

NoSuchRepositoryError::~NoSuchRepositoryError()
{
}

RepositoryName
NoSuchRepositoryError::name() const
{
    return _name;
}

struct AmbiguousPackageNameError::NameData
{
    std::string name;
    std::list<std::string> names;
};

AmbiguousPackageNameError::AmbiguousPackageNameError(const std::string & our_name,
        const std::shared_ptr<const Sequence<std::string> > & names) noexcept :
    Exception("Ambiguous package name '" + our_name + "' (candidates are " + join(names->begin(), names->end(), ", ") + ")"),
    _name_data(new NameData)
{
    _name_data->name = our_name;
    std::copy(names->begin(), names->end(), std::back_inserter(_name_data->names));
}

AmbiguousPackageNameError::AmbiguousPackageNameError(const AmbiguousPackageNameError & other) :
    Exception(other),
    _name_data(new NameData)
{
    _name_data->name = other._name_data->name;
    _name_data->names = other._name_data->names;
}

AmbiguousPackageNameError::~AmbiguousPackageNameError()
{
    delete _name_data;
}

AmbiguousPackageNameError::OptionsConstIterator
AmbiguousPackageNameError::begin_options() const
{
    return OptionsConstIterator(_name_data->names.begin());
}

AmbiguousPackageNameError::OptionsConstIterator
AmbiguousPackageNameError::end_options() const
{
    return OptionsConstIterator(_name_data->names.end());
}

const std::string &
AmbiguousPackageNameError::name() const
{
    return _name_data->name;
}

namespace paludis
{
    template class WrappedForwardIterator<AmbiguousPackageNameError::OptionsConstIteratorTag, const std::string>;
}
