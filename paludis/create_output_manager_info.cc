/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009 Ciaran McCreesh
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

#include <paludis/create_output_manager_info.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/exception.hh>
#include <ostream>

using namespace paludis;

#include <paludis/create_output_manager_info-se.cc>

namespace paludis
{
    template <>
    struct Implementation<CreateOutputManagerForPackageIDActionInfo>
    {
        const std::tr1::shared_ptr<const PackageID> id;
        const Action & action;
        const OutputExclusivity output_exclusivity;

        Implementation(const std::tr1::shared_ptr<const PackageID> & i,
                const Action & a, const OutputExclusivity e) :
            id(i),
            action(a),
            output_exclusivity(e)
        {
        }
    };

    template <>
    struct Implementation<CreateOutputManagerForRepositorySyncInfo>
    {
        const Repository & repo;
        const OutputExclusivity output_exclusivity;

        Implementation(const Repository & r, const OutputExclusivity e) :
            repo(r),
            output_exclusivity(e)
        {
        }
    };
}

CreateOutputManagerForPackageIDActionInfo::CreateOutputManagerForPackageIDActionInfo(
        const std::tr1::shared_ptr<const PackageID> & i,
        const Action & a,
        const OutputExclusivity e) :
    PrivateImplementationPattern<CreateOutputManagerForPackageIDActionInfo>(
            new Implementation<CreateOutputManagerForPackageIDActionInfo>(i, a, e))
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

OutputExclusivity
CreateOutputManagerForPackageIDActionInfo::output_exclusivity() const
{
    return _imp->output_exclusivity;
}

CreateOutputManagerForRepositorySyncInfo::CreateOutputManagerForRepositorySyncInfo(
        const Repository & r, const OutputExclusivity e) :
    PrivateImplementationPattern<CreateOutputManagerForRepositorySyncInfo>(
            new Implementation<CreateOutputManagerForRepositorySyncInfo>(r, e))
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

OutputExclusivity
CreateOutputManagerForRepositorySyncInfo::output_exclusivity() const
{
    return _imp->output_exclusivity;
}

template class PrivateImplementationPattern<CreateOutputManagerForRepositorySyncInfo>;
template class PrivateImplementationPattern<CreateOutputManagerForPackageIDActionInfo>;

