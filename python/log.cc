/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Piotr Jaroszyński <peper@gentoo.org>
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

#include <paludis_python.hh>

#include <paludis/util/log.hh>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

void PALUDIS_VISIBLE expose_log()
{
    enum_auto("LogLevel", last_ll);
    enum_auto("LogContext", last_lc);

    bp::class_<Log, boost::noncopyable>
        l("Log",
                "Singleton class that handles log messages.",
                bp::no_init
         );
    l.add_static_property("instance", bp::make_function(&Log::get_instance,
                bp::return_value_policy<bp::reference_existing_object>()),
            "[ro] Log\n"
            "Singleton instance."
            );
    void (Log::*message_ptr)(const LogLevel l, const LogContext c, const std::string & s) =
        &Log::message;
    l.def("message", message_ptr,
            "message(LogLevel, LogContext, string) -> None\n"
            "Log a message at the specified level."
         );
    l.add_property("log_level", &Log::log_level, &Log::set_log_level,
            "[rw] LogLevel\n"
            "Log level - only display messages of at least this level."
            );
    l.add_property("program_name", bp::object(), &Log::set_program_name,
            "[wo] string\n"
            "Program name"
            );
}
