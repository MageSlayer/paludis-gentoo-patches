/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Ciaran McCreesh
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

#include <paludis/resolver/job_state.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/make_shared_ptr.hh>

using namespace paludis;
using namespace paludis::resolver;

namespace paludis
{
    template <>
    struct Implementation<JobActiveState>
    {
        std::tr1::shared_ptr<OutputManager> output_manager;
    };

    template <>
    struct Implementation<JobSucceededState>
    {
        const std::tr1::shared_ptr<OutputManager> output_manager;

        Implementation(const std::tr1::shared_ptr<OutputManager> & m) :
            output_manager(m)
        {
        }
    };

    template <>
    struct Implementation<JobFailedState>
    {
        const std::tr1::shared_ptr<OutputManager> output_manager;

        Implementation(const std::tr1::shared_ptr<OutputManager> & m) :
            output_manager(m)
        {
        }
    };
}

JobState::~JobState()
{
}

JobActiveState::JobActiveState() :
    PrivateImplementationPattern<JobActiveState>(new Implementation<JobActiveState>)
{
}

JobActiveState::~JobActiveState()
{
}

void
JobActiveState::set_output_manager(const std::tr1::shared_ptr<OutputManager> & m)
{
    _imp->output_manager = m;
}

const std::tr1::shared_ptr<JobSucceededState>
JobActiveState::succeeded() const
{
    return make_shared_ptr(new JobSucceededState(_imp->output_manager));
}

const std::tr1::shared_ptr<JobFailedState>
JobActiveState::failed() const
{
    return make_shared_ptr(new JobFailedState(_imp->output_manager));
}

JobSucceededState::JobSucceededState(const std::tr1::shared_ptr<OutputManager> & m) :
    PrivateImplementationPattern<JobSucceededState>(new Implementation<JobSucceededState>(m))
{
}

JobSucceededState::~JobSucceededState()
{
}

const std::tr1::shared_ptr<OutputManager>
JobSucceededState::output_manager() const
{
    return _imp->output_manager;
}

JobFailedState::JobFailedState(const std::tr1::shared_ptr<OutputManager> & m) :
    PrivateImplementationPattern<JobFailedState>(new Implementation<JobFailedState>(m))
{
}

JobFailedState::~JobFailedState()
{
}

const std::tr1::shared_ptr<OutputManager>
JobFailedState::output_manager() const
{
    return _imp->output_manager;
}

