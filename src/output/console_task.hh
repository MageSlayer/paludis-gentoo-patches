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

#ifndef PALUDIS_GUARD_SRC_OUTPUT_CONSOLE_TASK_HH
#define PALUDIS_GUARD_SRC_OUTPUT_CONSOLE_TASK_HH 1

#include <iosfwd>
#include <string>
#include <paludis/util/attributes.hh>

namespace paludis
{
    class PALUDIS_VISIBLE ConsoleTask
    {
        protected:
            ConsoleTask();

        public:
            virtual ~ConsoleTask();

            ///\name Settings
            ///\{

            virtual int left_column_width() const;

            ///\}

            ///\name Output routines
            ///\{

            virtual std::ostream & output_stream() const;
            virtual std::ostream & output_xterm_stream() const;

            virtual void output_activity_start_message(const std::string &) const;
            virtual void output_activity_end_message() const;
            virtual void output_heading(const std::string &) const;
            virtual void output_xterm_title(const std::string &) const;
            virtual void output_starred_item(const std::string &, const unsigned indent = 0) const;
            virtual void output_starred_item_no_endl(const std::string &) const;
            virtual void output_unstarred_item(const std::string &) const;
            virtual void output_no_endl(const std::string &) const;
            virtual void output_endl() const;

            virtual void output_left_column(const std::string &, const unsigned indent = 0) const;
            virtual void output_right_column(const std::string &) const;

            ///\}

            ///\name Render routines
            ///\{

            virtual std::string render_as_package_name(const std::string &) const;
            virtual std::string render_as_repository_name(const std::string &) const;
            virtual std::string render_as_set_name(const std::string &) const;
            virtual std::string render_as_tag(const std::string &) const;
            virtual std::string render_as_unimportant(const std::string &) const;
            virtual std::string render_as_error(const std::string &) const;
            virtual std::string render_as_masked(const std::string &) const;
            virtual std::string render_as_visible(const std::string &) const;
            virtual std::string render_as_slot_name(const std::string &) const;
            virtual std::string render_as_update_mode(const std::string &) const;
            virtual std::string render_plural(int count, const std::string &, const std::string &) const;

            ///\}
    };
}

#endif
