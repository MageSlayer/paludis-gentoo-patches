/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_LOG_HH
#define PALUDIS_GUARD_PALUDIS_LOG_HH 1

#include <paludis/util/stringify.hh>
#include <paludis/util/singleton.hh>
#include <paludis/util/pimp.hh>
#include <iosfwd>
#include <string>

/** \file
 * Declarations for Log and related classes.
 *
 * \ingroup g_log
 *
 * \section Examples
 *
 * - None at this time.
 */

namespace paludis
{
#include <paludis/util/log-se.hh>

    class LogMessageHandler;

    /**
     * Singleton class that handles log messages.
     *
     * \ingroup g_log
     */
    class PALUDIS_VISIBLE Log :
        public Singleton<Log>,
        private Pimp<Log>
    {
        friend class Singleton<Log>;
        friend class LogMessageHandler;

        private:
            Log();

            void _message(const std::string & id, const LogLevel, const LogContext, const std::string &);

        public:
            /**
             * Destructor, to be called only by our InstantiationPolicy.
             */
            ~Log();

            /**
             * Only display messages of at least this level.
             */
            void set_log_level(const LogLevel);

            /**
             * Fetch the current log level.
             */
            LogLevel log_level() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Log a message.
             *
             * The return value can be appended to using
             * LogMessageHandler::operator<<(). When the return value is
             * destroyed (that is to say, at the end of the statement), the log
             * message is written.
             */
            LogMessageHandler message(const std::string & id,
                    const LogLevel, const LogContext) PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Change the log stream.
             */
            void set_log_stream(std::ostream * const);

            /**
             * Set our program name.
             */
            void set_program_name(const std::string &);

            /**
             * Finish any pending writes.
             */
            void complete_pending() const;
    };

    /**
     * Used by Log::message().
     *
     * \see Log
     * \ingroup g_log
     */
    class PALUDIS_VISIBLE LogMessageHandler
    {
        friend LogMessageHandler Log::message(const std::string &, const LogLevel, const LogContext);

        private:
            Log * _log;
            std::string _id;
            std::string _message;
            LogLevel _log_level;
            LogContext _log_context;

            LogMessageHandler(const LogMessageHandler &);
            LogMessageHandler(Log * const, const std::string &, const LogLevel, const LogContext);
            void operator= (const LogMessageHandler &);

            void _append(const std::string & s);

        public:
            ///\name Basic operations
            ///\{

            ~LogMessageHandler();

            ///\}

            /**
             * Append some text to our message.
             */
            template <typename T_>
            LogMessageHandler &
            operator<< (const T_ & t)
            {
                _append(stringify(t));
                return *this;
            }
    };
}

#endif
