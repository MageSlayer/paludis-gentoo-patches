/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Danny van Dyk <kugelfang@gentoo.org>
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
#include <list>

using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<StageBuilderTask> :
        InternalCounted<Implementation<StageBuilderTask> >
    {
        std::list<StageBase::ConstPointer> stages;

        const StageOptions options;

        Implementation(const StageOptions & o) :
            options(o)
        {
        }
    };
}

#include <paludis/tasks/stage_options-sr.cc>

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
StageBuilderTask::queue_stage(StageBase::ConstPointer p)
{
    Context context("When queuing stage in build list:");
    _imp->stages.push_back(p);
}

StageBuilderTask::StageIterator
StageBuilderTask::begin_stages() const
{
    return StageIterator(_imp->stages.begin());
}

StageBuilderTask::StageIterator
StageBuilderTask::end_stages() const
{
    return StageIterator(_imp->stages.end());
}

void
StageBuilderTask::execute()
{
    Context context("When executing stage builder task:");

#if 0
    if (_imp->stages.empty())
        throw NoStageListError(); //TODO: Needed?
#endif

    on_build_all_pre();

    for (std::list<StageBase::ConstPointer>::const_iterator
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

            (*s)->build(_imp->options);

            if (! _imp->options.pretend)
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

