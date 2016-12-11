/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_OUTPUT_MANAGER_HH
#define PALUDIS_GUARD_PALUDIS_OUTPUT_MANAGER_HH 1

#include <paludis/output_manager-fwd.hh>
#include <paludis/util/attributes.hh>
#include <iosfwd>

namespace paludis
{
    class PALUDIS_VISIBLE OutputManager
    {
        public:
            OutputManager() = default;
            virtual ~OutputManager() noexcept(false) = 0;

            OutputManager(const OutputManager &) = delete;
            OutputManager & operator= (const OutputManager &) = delete;

            virtual std::ostream & stdout_stream() PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
            virtual std::ostream & stderr_stream() PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * An out of band message that might want to be logged or handled
             * in a special way.
             *
             * The caller must still also display the message to
             * stdout_stream() as appropriate.
             */
            virtual void message(const MessageType, const std::string &) = 0;

            /**
             * Clients may call this method every few seconds when running
             * multiple processes.
             *
             * This is used to display ongoing buffered messages without mixing
             * output from multiple processes.
             */
            virtual void flush() = 0;

            /**
             * Do we want to flush?
             *
             * Provides a way for clients to avoid having to call flush() with
             * a prefixed header when there's no output waiting.
             */
            virtual bool want_to_flush() const = 0;

            /**
             * Called if an action succeeds. This can be used to, for example,
             * unlink the files behind a to-disk logged output manager.
             *
             * If an OutputManager is destroyed without having had this method
             * called, it should assume failure. This might mean keeping rather
             * than removing log files, for example.
             *
             * Further messages and output may occur even after a call to this
             * method.
             *
             * Calls to this method are done by the caller, not by whatever
             * carries out the action in question.
             *
             * If ignore_succeeded() has previously been called, does nothing.
             */
            virtual void succeeded() = 0;

            /**
             * Instructs the output manager to ignore future calls to
             * succeeded().
             *
             * Typically this is used to force log files to be kept even if
             * an error has occurred, if the error does not trigger the usual
             * failure mechanisms.
             *
             * \since 0.59
             */
            virtual void ignore_succeeded() = 0;

            /**
             * May be called to indicate that no further output or messages
             * will occur, allowing for files to be closed off etc.
             *
             * Summary messages are shown when the output manager is
             * destructed, not when this method is called.
             *
             * If this method and succeeded are both to be called, succeeded
             * must be called first.
             */
            virtual void nothing_more_to_come() = 0;
    };
}

#endif
