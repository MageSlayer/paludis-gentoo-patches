/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh
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

#include "console_task.hh"
#include "colour.hh"
#include <iostream>
#include <iomanip>

using namespace paludis;

ConsoleTask::ConsoleTask()
{
}

ConsoleTask::~ConsoleTask()
{
}

void
ConsoleTask::output_activity_start_message(const std::string & s) const
{
    output_stream() << s << "..." << std::flush;
}

void
ConsoleTask::output_activity_end_message() const
{
    output_stream() << std::endl;
}

void
ConsoleTask::output_heading(const std::string & s) const
{
    output_stream() << std::endl << colour(cl_heading, s) << std::endl << std::endl;
}

void
ConsoleTask::output_xterm_title(const std::string & s) const
{
    output_xterm_stream() << xterm_title(s);
}

void
ConsoleTask::output_starred_item(const std::string & s, const unsigned indent) const
{
    if (0 != indent)
        output_stream() << std::string(2 * indent, ' ') << "* " << s << std::endl;
    else
        output_stream() << "* " << s << std::endl;
}

void
ConsoleTask::output_starred_item_no_endl(const std::string & s) const
{
    output_stream() << "* " << s;
}

void
ConsoleTask::output_unstarred_item(const std::string & s) const
{
    output_stream() << s << std::endl;
}

void
ConsoleTask::output_no_endl(const std::string & s) const
{
    output_stream() << s;
}

void
ConsoleTask::output_endl() const
{
    output_stream() << std::endl;
}

std::ostream &
ConsoleTask::output_stream() const
{
    return std::cout;
}

std::ostream &
ConsoleTask::output_xterm_stream() const
{
    return std::cerr;
}

std::string
ConsoleTask::render_as_package_name(const std::string & s) const
{
    return colour(cl_package_name, s);
}

std::string
ConsoleTask::render_as_repository_name(const std::string & s) const
{
    return colour(cl_repository_name, s);
}

std::string
ConsoleTask::render_as_set_name(const std::string & s) const
{
    return colour(cl_package_name, s);
}

std::string
ConsoleTask::render_as_tag(const std::string & s) const
{
    return colour(cl_tag, s);
}

std::string
ConsoleTask::render_as_unimportant(const std::string & s) const
{
    return colour(cl_unimportant, s);
}

std::string
ConsoleTask::render_as_slot_name(const std::string & s) const
{
    return colour(cl_slot, s);
}

std::string
ConsoleTask::render_as_update_mode(const std::string & s) const
{
    return colour(cl_updatemode, s);
}

std::string
ConsoleTask::render_as_error(const std::string & s) const
{
    return colour(cl_error, s);
}

std::string
ConsoleTask::render_as_masked(const std::string & s) const
{
    return colour(cl_masked, s);
}

std::string
ConsoleTask::render_as_visible(const std::string & s) const
{
    return colour(cl_visible, s);
}

std::string
ConsoleTask::render_plural(int c, const std::string & s, const std::string & p) const
{
    return 1 == c ? s : p;
}

void
ConsoleTask::output_left_column(const std::string & s) const
{
    output_stream() << "    " << std::setw(left_column_width()) << std::left << s << " ";
}

void
ConsoleTask::output_right_column(const std::string & s) const
{
    output_stream() << s << std::endl;
}

int
ConsoleTask::left_column_width() const
{
    return 24;
}

