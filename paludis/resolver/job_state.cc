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
#include <paludis/serialise-impl.hh>

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

const std::tr1::shared_ptr<JobState>
JobState::deserialise(Deserialisation & d)
{
    if (d.class_name() == "JobPendingState")
        return JobPendingState::deserialise(d);
    else if (d.class_name() == "JobActiveState")
        return JobActiveState::deserialise(d);
    else if (d.class_name() == "JobSucceededState")
        return JobSucceededState::deserialise(d);
    else if (d.class_name() == "JobFailedState")
        return JobFailedState::deserialise(d);
    else if (d.class_name() == "JobSkippedState")
        return JobSkippedState::deserialise(d);
    else
        throw InternalError(PALUDIS_HERE, "unknown class '" + stringify(d.class_name()) + "'");
}

const std::tr1::shared_ptr<JobPendingState>
JobPendingState::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "JobPendingState");
    return make_shared_ptr(new JobPendingState);
}

void
JobPendingState::serialise(Serialiser & s) const
{
    s.object("JobPendingState")
        ;
}

const std::tr1::shared_ptr<JobSkippedState>
JobSkippedState::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "JobSkippedState");
    return make_shared_ptr(new JobSkippedState);
}

void
JobSkippedState::serialise(Serialiser & s) const
{
    s.object("JobSkippedState")
        ;
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

const std::tr1::shared_ptr<JobActiveState>
JobActiveState::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "JobActiveState");
    return make_shared_ptr(new JobActiveState());
}

void
JobActiveState::serialise(Serialiser & s) const
{
    s.object("JobActiveState")
        ;
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

const std::tr1::shared_ptr<JobSucceededState>
JobSucceededState::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "JobSucceededState");
    return make_shared_ptr(new JobSucceededState(make_null_shared_ptr()));
}

void
JobSucceededState::serialise(Serialiser & s) const
{
    s.object("JobSucceededState")
        ;
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

const std::tr1::shared_ptr<JobFailedState>
JobFailedState::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "JobFailedState");
    return make_shared_ptr(new JobFailedState(make_null_shared_ptr()));
}

void
JobFailedState::serialise(Serialiser & s) const
{
    s.object("JobFailedState")
        ;
}

