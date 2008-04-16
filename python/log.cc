/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Piotr Jaroszyński
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

#include <python/paludis_python.hh>

#include <paludis/util/log.hh>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

namespace
{
    void log_message(Log * const log, const std::string & id, const LogLevel l, const LogContext c, const std::string & s)
    {
        log->message(id, l, c) << s;
    }
}

void expose_log()
{
    /**
     * Enums
     */
    enum_auto("LogLevel", last_ll,
            "Specifies the level of a log message.");
    enum_auto("LogContext", last_lc,
            "Specifies whether a log message has context.");

    /**
     * Log
     */
    bp::class_<Log, boost::noncopyable>
        (
         "Log",
                "Singleton class that handles log messages.",
                bp::no_init
        )
        .add_static_property("instance", bp::make_function(&Log::get_instance,
                    bp::return_value_policy<bp::reference_existing_object>()),
                "[ro] Log\n"
                "Singleton instance."
                )

        .def("message", log_message,
                "message(string, LogLevel, LogContext, string) -> None\n"
                "Log a message at the specified leve."
            )

        .add_property("log_level", &Log::log_level, &Log::set_log_level,
                "[rw] LogLevel\n"
                "Log level - only display messages of at least this leve."
                )

        .add_property("program_name", bp::object(), &Log::set_program_name,
                "[wo] string\n"
                "Program name"
                )
        ;
}
