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

#ifndef PALUDIS_GUARD_SRC_CONTRARIUS_STAGE_BUILDER_HH
#define PALUDIS_GUARD_SRC_CONTRARIUS_STAGE_BUILDER_HH 1

#include <paludis/tasks/stage_builder_task.hh>
#include <src/contrarius/stage.hh>
#include <string>

namespace paludis
{
    class OurStageBuilderTask :
        public StageBuilderTask
    {
        public:
            OurStageBuilderTask(const StageOptions & o) :
                StageBuilderTask(o)
            {
            }

            virtual void on_build_all_pre();
            virtual void on_build_pre(StageBase::ConstPointer);
            virtual void on_build_post(StageBase::ConstPointer);
            virtual void on_build_fail(StageBase::ConstPointer, const StageBuildError &);
            virtual void on_build_succeed(StageBase::ConstPointer);
            virtual void on_build_skipped(StageBase::ConstPointer);
            virtual void on_build_all_post();
    };
}

#endif
