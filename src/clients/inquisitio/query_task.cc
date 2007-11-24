/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh
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

#include "query_task.hh"
#include "command_line.hh"

using namespace inquisitio;
using namespace paludis;

InquisitioQueryTask::InquisitioQueryTask(const Environment * const env) :
    ConsoleQueryTask(env)
{
}

bool
InquisitioQueryTask::want_deps() const
{
    return CommandLine::get_instance()->a_show_dependencies.specified();
}

bool
InquisitioQueryTask::want_raw() const
{
    return CommandLine::get_instance()->a_show_metadata.specified();
}

bool
InquisitioQueryTask::want_compact() const
{
    return CommandLine::get_instance()->a_compact.specified();
}

bool
InquisitioQueryTask::want_authors() const
{
    return CommandLine::get_instance()->a_show_authors.specified();
}

