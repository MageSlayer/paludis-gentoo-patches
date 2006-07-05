/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#include <iostream>
#include <paludis/util/log.hh>

/** \file
 * Implementation for Log.
 *
 * \ingroup grplog
 */

using namespace paludis;

namespace paludis
{
    /**
     * Implementation data for Log.
     *
     * \ingroup grplog
     */
    template<>
    struct Implementation<Log> :
        InternalCounted<Implementation<Log> >
    {
        /// Current log level
        LogLevel log_level;

        /// Current output stream
        std::ostream * stream;

        /// Program name
        std::string program_name;
    };
}

Log::Log() :
    PrivateImplementationPattern<Log>(new Implementation<Log>)
{
    _imp->log_level = initial_ll;
    _imp->stream = &std::cerr;
    _imp->program_name = "paludis";
}

Log::~Log()
{
}

void
Log::set_log_level(const LogLevel l)
{
    _imp->log_level = l;
}

LogLevel
Log::log_level() const
{
    return _imp->log_level;
}

void
Log::message(const LogLevel l, const LogContext c, const std::string & s)
{
    if (l >= _imp->log_level)
    {
        *_imp->stream << _imp->program_name << "@" << ::time(0) << ": ";
        do
        {
            switch (l)
            {
                case ll_debug:
                    *_imp->stream << "[DEBUG] ";
                    continue;

                case ll_qa:
                    *_imp->stream << "[QA] ";
                    continue;

                case ll_warning:
                    *_imp->stream << "[WARNING] ";
                    continue;

                case ll_silent:
                    throw InternalError(PALUDIS_HERE, "ll_silent used for a message");

                case last_ll:
                    break;
            }

            throw InternalError(PALUDIS_HERE, "Bad value for log_level");

        } while (false);

        if (lc_context == c)
            *_imp->stream << Context::backtrace(" ") << s << std::endl;
        else
            *_imp->stream << s << std::endl;
    }
}

void
Log::set_log_stream(std::ostream * const s)
{
    _imp->stream = s;
}

std::string
Log::log_level_string() const
{
    switch (Log::get_instance()->log_level())
    {
        case ll_qa:
            return "qa";

        case ll_warning:
            return "warning";

        case ll_debug:
            return "debug";

        case ll_silent:
            return "silent";

        case last_ll:
            ;
    };

    throw InternalError(PALUDIS_HERE, "Bad log level");
}

void
Log::set_program_name(const std::string & s)
{
    _imp->program_name = s;
}
