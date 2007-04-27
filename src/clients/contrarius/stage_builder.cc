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

#include <paludis/tasks/stage_builder_task.hh>
#include <paludis/environment.hh>
#include <src/clients/contrarius/stage.hh>
#include <src/clients/contrarius/stage_builder.hh>
#include <src/output/colour.hh>

#include <iostream>
#include <list>
#include <string>

using namespace paludis;
using std::cout;
using std::endl;

void
OurStageBuilderTask::on_build_all_pre()
{
    cout << endl << colour(cl_heading, "These stages will be built:")
            << endl << endl;

    int num_stages(0), num_rebuilds(0);
    for (StageIterator s(begin_stages()), s_end(end_stages()) ; s != s_end ; s++)
    {
        cout << "o " << colour(cl_key_name, (*s)->short_name()) << endl
                << "  " << colour(cl_tag, (*s)->description()) << endl;
        ++num_stages;
        if ((*s)->is_rebuild())
            ++num_rebuilds;
    }

    cout << endl << "Total: " << num_stages << " Stage" << (num_stages > 1 ? "s" : "")
            << " (" << num_rebuilds << " rebuild)" << endl;

    cout << endl;
}

void
OurStageBuilderTask::on_build_pre(std::tr1::shared_ptr<const StageBase> s)
{
    cout << colour(cl_heading, "Contents of stage ")
        << colour(cl_stage_short_name, s->short_name()) << endl;
}

void
OurStageBuilderTask::on_build_post(std::tr1::shared_ptr<const StageBase>)
{
}

void
OurStageBuilderTask::on_build_fail(std::tr1::shared_ptr<const StageBase> s, const StageBuildError & e)
{
    cout << "Build of stage '" << s->short_name() << "' failed:" << endl;
    cout << "Error: " << e.message() << endl << endl;
    throw;
}

void
OurStageBuilderTask::on_build_succeed(std::tr1::shared_ptr<const StageBase> s)
{
    cout << "Build of stage '" << s->short_name() << "' succeeded." << endl << endl;
}

void
OurStageBuilderTask::on_build_skipped(std::tr1::shared_ptr<const StageBase> s)
{
    cout << "Skipped rebuild of stage '" << s->short_name() << "'" << endl << endl;
}

void
OurStageBuilderTask::on_build_all_post()
{
    cout << endl;
}
