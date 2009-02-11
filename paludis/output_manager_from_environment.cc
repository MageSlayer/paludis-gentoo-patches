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

#include <paludis/output_manager_from_environment.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/environment.hh>
#include <paludis/create_output_manager_info.hh>

using namespace paludis;

namespace paludis
{
    template <>
    struct Implementation<OutputManagerFromEnvironment>
    {
        const Environment * const env;
        const std::tr1::shared_ptr<const PackageID> id;
        const OutputExclusivity output_exclusivity;

        std::tr1::shared_ptr<OutputManager> result;

        Implementation(const Environment * const e, const std::tr1::shared_ptr<const PackageID> & i,
                const OutputExclusivity x) :
            env(e),
            id(i),
            output_exclusivity(x)
        {
        }
    };
}

OutputManagerFromEnvironment::OutputManagerFromEnvironment(
        const Environment * const e,
        const std::tr1::shared_ptr<const PackageID> & i,
        const OutputExclusivity x) :
    PrivateImplementationPattern<OutputManagerFromEnvironment>(new Implementation<OutputManagerFromEnvironment>(e, i, x))
{
}

OutputManagerFromEnvironment::~OutputManagerFromEnvironment()
{
}

const std::tr1::shared_ptr<OutputManager>
OutputManagerFromEnvironment::operator() (const Action & a)
{
    if (! _imp->result)
        _imp->result = _imp->env->create_output_manager(CreateOutputManagerForPackageIDActionInfo(_imp->id, a, _imp->output_exclusivity));
    return _imp->result;
}

const std::tr1::shared_ptr<OutputManager>
OutputManagerFromEnvironment::output_manager_if_constructed()
{
    return _imp->result;
}

template class PrivateImplementationPattern<OutputManagerFromEnvironment>;

