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
#include <list>

using namespace paludis;
using namespace paludis::resolver;

JobState::~JobState()
{
}

namespace paludis
{
    template <>
    struct Implementation<JobPendingState>
    {
        const std::tr1::shared_ptr<const Job> job;

        Implementation(const std::tr1::shared_ptr<const Job> & j) :
            job(j)
        {
        }
    };

    template <>
    struct Implementation<JobSucceededState>
    {
        const std::tr1::shared_ptr<const Job> job;
        std::list<std::tr1::shared_ptr<OutputManager> > output_managers;

        Implementation(const std::tr1::shared_ptr<const Job> & j) :
            job(j)
        {
        }
    };

    template <>
    struct Implementation<JobFailedState>
    {
        const std::tr1::shared_ptr<const Job> job;
        std::list<std::tr1::shared_ptr<OutputManager> > output_managers;

        Implementation(const std::tr1::shared_ptr<const Job> & j) :
            job(j)
        {
        }
    };

    template <>
    struct Implementation<JobSkippedState>
    {
        const std::tr1::shared_ptr<const Job> job;

        Implementation(const std::tr1::shared_ptr<const Job> & j) :
            job(j)
        {
        }
    };
}

JobPendingState::JobPendingState(const std::tr1::shared_ptr<const Job> & j) :
    PrivateImplementationPattern<JobPendingState>(new Implementation<JobPendingState>(j))
{
}

JobPendingState::~JobPendingState()
{
}

const std::tr1::shared_ptr<const Job>
JobPendingState::job() const
{
    return _imp->job;
}

const std::string
JobPendingState::state_name() const
{
    return "pending";
}

JobSucceededState::JobSucceededState(const std::tr1::shared_ptr<const Job> & j) :
    PrivateImplementationPattern<JobSucceededState>(new Implementation<JobSucceededState>(j))
{
}

JobSucceededState::~JobSucceededState()
{
}

const std::tr1::shared_ptr<const Job>
JobSucceededState::job() const
{
    return _imp->job;
}

const std::string
JobSucceededState::state_name() const
{
    return "succeeded";
}

void
JobSucceededState::add_output_manager(const std::tr1::shared_ptr<OutputManager> & o)
{
    _imp->output_managers.push_back(o);
}

JobFailedState::JobFailedState(const std::tr1::shared_ptr<const Job> & j) :
    PrivateImplementationPattern<JobFailedState>(new Implementation<JobFailedState>(j))
{
}

JobFailedState::~JobFailedState()
{
}

const std::tr1::shared_ptr<const Job>
JobFailedState::job() const
{
    return _imp->job;
}

void
JobFailedState::add_output_manager(const std::tr1::shared_ptr<OutputManager> & o)
{
    _imp->output_managers.push_back(o);
}

const std::string
JobFailedState::state_name() const
{
    return "failed";
}

JobSkippedState::JobSkippedState(const std::tr1::shared_ptr<const Job> & j) :
    PrivateImplementationPattern<JobSkippedState>(new Implementation<JobSkippedState>(j))
{
}

JobSkippedState::~JobSkippedState()
{
}

const std::tr1::shared_ptr<const Job>
JobSkippedState::job() const
{
    return _imp->job;
}

const std::string
JobSkippedState::state_name() const
{
    return "skipped";
}

template class PrivateImplementationPattern<JobPendingState>;
template class PrivateImplementationPattern<JobSucceededState>;
template class PrivateImplementationPattern<JobFailedState>;
template class PrivateImplementationPattern<JobSkippedState>;

