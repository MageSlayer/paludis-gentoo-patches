/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Danny van Dyk
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

#include "stage_builder_task.hh"

#include <paludis/environment.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <list>

using namespace paludis;

template class WrappedForwardIterator<StageBuilderTask::StageConstIteratorTag,
         const std::tr1::shared_ptr<const StageBase> >;

namespace paludis
{
    template<>
    struct Implementation<StageBuilderTask>
    {
        std::list<std::tr1::shared_ptr<const StageBase> > stages;

        const StageOptions options;

        Implementation(const StageOptions & o) :
            options(o)
        {
        }
    };
}

#include <paludis/stage_options-sr.cc>

StageBuildError::StageBuildError(const std::string & m) throw () :
    Exception(m)
{
}

StageBase::~StageBase()
{
}

StageBuilderTask::StageBuilderTask(const StageOptions & o) :
    PrivateImplementationPattern<StageBuilderTask>(new Implementation<StageBuilderTask>(o))
{
}

StageBuilderTask::~StageBuilderTask()
{
}

void
StageBuilderTask::queue_stage(const std::tr1::shared_ptr<const StageBase> & p)
{
    Context context("When queuing stage in build list:");
    _imp->stages.push_back(p);
}

StageBuilderTask::StageConstIterator
StageBuilderTask::begin_stages() const
{
    return StageConstIterator(_imp->stages.begin());
}

StageBuilderTask::StageConstIterator
StageBuilderTask::end_stages() const
{
    return StageConstIterator(_imp->stages.end());
}

void
StageBuilderTask::execute()
{
    Context context("When executing stage builder task:");

    on_build_all_pre();

    for (std::list<std::tr1::shared_ptr<const StageBase> >::const_iterator
            s(_imp->stages.begin()), s_end(_imp->stages.end()) ;
            s != s_end ; ++s)
    {
        Context context_local("When building stage '" + (*s)->short_name() + "':");

        on_build_pre(*s);

        try
        {
            if (((*s)->is_rebuild()) && (! _imp->options.always_rebuild))
            {
                on_build_skipped(*s);
                continue;
            }

            if (! (*s)->build(_imp->options))
                throw StageBuildError("Failed building stage '" + (*s)->short_name() + "'");

            if (_imp->options.pretend)
                continue;

            on_build_succeed(*s);
        }
        catch (const StageBuildError & e)
        {
            on_build_fail(*s, e);
        }


        on_build_post(*s);
    }

    on_build_all_post();
}

