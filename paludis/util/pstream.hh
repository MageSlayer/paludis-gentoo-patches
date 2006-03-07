/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_PSTREAM_HH
#define PALUDIS_GUARD_PALUDIS_PSTREAM_HH 1

#include <cstdio>
#include <istream>
#include <limits>
#include <paludis/util/exception.hh>
#include <paludis/util/instantiation_policy.hh>
#include <streambuf>
#include <string>

/** \file
 * Declarations for the PStream and PStreamInBuf classes, and related
 * utilities.
 *
 * \ingroup PStream
 * \ingroup Exception
 */

namespace paludis
{
    /**
     * Thrown if a PStream or PStreamInBuf encounters an error.
     *
     * \ingroup PStream
     * \ingroup Exception
     */
    class PStreamError : public Exception
    {
        public:
            /**
             * Constructor.
             */
            PStreamError(const std::string & message) throw ();
    };

    /**
     * Input buffer class for a process, invoked using popen(3).
     *
     * Bidirectional I/O isn't supported since we haven't needed it yet, and
     * because popen on Linux is unidirectional.
     *
     * See \ref TCppSL Ch. 13.13 for what we're doing here. The buffer code is
     * based upon the "io/inbuf1.hpp" example in section 13.13.3.
     *
     * \ingroup PStream
     */
    class PStreamInBuf :
        public std::streambuf,
        private InstantiationPolicy<PStreamInBuf, instantiation_method::NonCopyableTag>
    {
        private:
            const std::string _command;

            int _exit_status;

        protected:
            /**
             * Our file descriptor.
             */
            FILE * fd;

            /**
             * At most how many characters can we put back?
             */
            static const int putback_size = std::numeric_limits<unsigned>::digits >> 3;

            /**
             * How large is our internal buffer?
             */
            static const int buffer_size = 3 * putback_size;

            /**
             * Internal buffer.
             */
            char buffer[buffer_size];

            /**
             * Called when an underflow occurs.
             */
            virtual int_type underflow();

        public:
            /**
             * Constructor.
             *
             * \param command The command to run. See PStream for discussion.
             */
            PStreamInBuf(const std::string & command);

            /**
             * Destructor.
             */
            ~PStreamInBuf();

            /**
             * What was our command?
             */
            const std::string & command() const
            {
                return _command;
            }

            /**
             * What is our exit status?
             */
            int exit_status();
    };

    /**
     * For internal use by PStream classes.
     *
     * \ingroup PStream
     */
    namespace pstream_internals
    {
        /**
         * Avoid base from member issues for PStream.
         *
         * \ingroup PStream
         */
        struct PStreamInBufBase :
            private paludis::InstantiationPolicy<PStreamInBufBase, instantiation_method::NonCopyableTag>
        {
            /**
             * Our buffer.
             */
            PStreamInBuf buf;

            /**
             * Constructor.
             */
            PStreamInBufBase(const std::string & command) :
                buf(command)
            {
            }
        };
    }

    /**
     * A PStream class is a standard input stream class whose contents comes
     * from the output of an executed command.
     *
     * \ingroup PStream
     */
    class PStream :
        private InstantiationPolicy<PStream, instantiation_method::NonCopyableTag>,
        protected pstream_internals::PStreamInBufBase,
        public std::istream
    {
        public:
            /**
             * Constructor.
             *
             * \param command The command to execute. PATH is used, so there is
             * usually no need to specify a full path. Arguments can be passed
             * as part of the command.
             */
            PStream(const std::string & command) :
                PStreamInBufBase(command),
                std::istream(&buf)
            {
            }

            /**
             * What is our exit status?
             */
            int exit_status()
            {
                return buf.exit_status();
            }
    };
}

#endif
