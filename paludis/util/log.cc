/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2010, 2011 Ciaran McCreesh
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
#include <exception>
#include <paludis/util/log.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/singleton-impl.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/mutex.hh>
#include "config.h"

#ifdef __linux__
#  include <sys/time.h>
#  include <sys/resource.h>
#  include <unistd.h>
#  include <sys/syscall.h>
#endif

using namespace paludis;

#include <paludis/util/log-se.cc>

template class Singleton<Log>;

namespace paludis
{
    template<>
    struct Imp<Log>
    {
        Mutex mutex;
        LogLevel log_level;
        std::ostream * stream;
        std::string program_name;
        std::string previous_context;

        Imp() :
            log_level(ll_qa),
            stream(&std::cerr),
            program_name("paludis")
        {
        }

        void message(const std::string id, const LogLevel l, const LogContext c, const std::string cs, const std::string s)
        {
            if (l >= log_level)
            {
                *stream << program_name << "@" << ::time(0) << ": ";
                do
                {
                    switch (l)
                    {
                        case ll_debug:
                            *stream << "[DEBUG " << id << "] ";
                            continue;

                        case ll_qa:
                            *stream << "[QA " << id << "] ";
                            continue;

                        case ll_warning:
                            *stream << "[WARNING " << id << "] ";
                            continue;

                        case ll_silent:
                            throw InternalError(PALUDIS_HERE, "ll_silent used for a message");

                        case last_ll:
                            break;
                    }

                    throw InternalError(PALUDIS_HERE, "Bad value for log_level");

                } while (false);

                if (lc_context == c)
                {
                    if (previous_context == cs)
                        *stream << "(same context) " << s << std::endl;
                    else
                        *stream << cs << s << std::endl;
                    previous_context = cs;
                }
                else
                    *stream << s << std::endl;
            }
        }

        void set_log_level(const LogLevel l)
        {
            log_level = l;
        }

        void get_log_level(LogLevel & l) const
        {
            l = log_level;
        }

        void set_program_name(const std::string & s)
        {
            program_name = s;
        }

        void set_log_stream(std::ostream * const s)
        {
            stream = s;
        }
    };
}

Log::Log() :
    _imp()
{
}

Log::~Log()
{
}

void
Log::set_log_level(const LogLevel l)
{
    Lock lock(_imp->mutex);
    _imp->log_level = l;
}

LogLevel
Log::log_level() const
{
    return _imp->log_level;
}

void
Log::_message(const std::string & id, const LogLevel l, const LogContext c, const std::string & s)
{
    Lock lock(_imp->mutex);

    if (lc_context == c)
        _imp->message(id, l, c,
#ifdef __linux__
                "In thread ID '" + stringify(syscall(SYS_gettid)) + "':\n  ... " +
#else
#  warning "Don't know how to get a thread ID on your platform"
#endif
                Context::backtrace("\n  ... "), s);
    else
        _imp->message(id, l, c, "", s);
}

LogMessageHandler::LogMessageHandler(const LogMessageHandler & o) :
    _id(o._id),
    _message(o._message),
    _log_level(o._log_level),
    _log_context(o._log_context)
{
}

LogMessageHandler
Log::message(const std::string & id, const LogLevel l, const LogContext c)
{
    return LogMessageHandler(this, id, l, c);
}

void
Log::set_log_stream(std::ostream * const s)
{
    Lock lock(_imp->mutex);
    _imp->stream = s;
}

void
Log::set_program_name(const std::string & s)
{
    Lock lock(_imp->mutex);
    _imp->program_name = s;
}

LogMessageHandler::LogMessageHandler(Log * const ll, const std::string & id, const LogLevel l, const LogContext c) :
    _log(ll),
    _id(id),
    _log_level(l),
    _log_context(c)
{
}

void
LogMessageHandler::_append(const std::string & s)
{
    _message.append(s);
}

LogMessageHandler::~LogMessageHandler()
{
    if (! std::uncaught_exception() && ! _message.empty())
        _log->_message(_id, _log_level, _log_context, _message);
}

template class Pimp<Log>;

