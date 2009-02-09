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

#include <paludis/environment.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/wrapped_output_iterator-impl.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>

using namespace paludis;

template class WrappedForwardIterator<Sequence<std::string>::ConstIteratorTag, const std::string>;
template class WrappedOutputIterator<Sequence<std::string>::InserterTag, std::string>;

Environment::~Environment()
{
}

namespace paludis
{
    template <>
    struct Implementation<CreateOutputManagerForPackageIDActionInfo>
    {
        const std::tr1::shared_ptr<const PackageID> id;
        const Action & action;

        Implementation(const std::tr1::shared_ptr<const PackageID> & i,
                const Action & a) :
            id(i),
            action(a)
        {
        }
    };

    template <>
    struct Implementation<CreateOutputManagerForRepositorySyncInfo>
    {
        const Repository & repo;

        Implementation(const Repository & r) :
            repo(r)
        {
        }
    };
}

CreateOutputManagerForPackageIDActionInfo::CreateOutputManagerForPackageIDActionInfo(
        const std::tr1::shared_ptr<const PackageID> & i,
        const Action & a) :
    PrivateImplementationPattern<CreateOutputManagerForPackageIDActionInfo>(
            new Implementation<CreateOutputManagerForPackageIDActionInfo>(i, a))
{
}

CreateOutputManagerForPackageIDActionInfo::~CreateOutputManagerForPackageIDActionInfo()
{
}

const std::tr1::shared_ptr<const PackageID>
CreateOutputManagerForPackageIDActionInfo::package_id() const
{
    return _imp->id;
}

const Action &
CreateOutputManagerForPackageIDActionInfo::action() const
{
    return _imp->action;
}

CreateOutputManagerForRepositorySyncInfo::CreateOutputManagerForRepositorySyncInfo(
        const Repository & r) :
    PrivateImplementationPattern<CreateOutputManagerForRepositorySyncInfo>(
            new Implementation<CreateOutputManagerForRepositorySyncInfo>(r))
{
}

CreateOutputManagerForRepositorySyncInfo::~CreateOutputManagerForRepositorySyncInfo()
{
}

const Repository &
CreateOutputManagerForRepositorySyncInfo::repository() const
{
    return _imp->repo;
}

template class PrivateImplementationPattern<CreateOutputManagerForRepositorySyncInfo>;
template class PrivateImplementationPattern<CreateOutputManagerForPackageIDActionInfo>;

