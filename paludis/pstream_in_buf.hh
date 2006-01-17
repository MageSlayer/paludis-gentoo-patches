/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@gentoo.org>
 *
 * This file is part of the Paludis package manager. Paludis is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
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

#ifndef PALUDIS_GUARD_PALUDIS_PSTREAM_IN_BUF_HH
#define PALUDIS_GUARD_PALUDIS_PSTREAM_IN_BUF_HH 1

#include <paludis/instantiation_policy.hh>
#include <streambuf>
#include <limits>
#include <string>
#include <cstdio>

/** \file
 * Declarations for the PStreamInBuf class.
 *
 * \ingroup PStream
 */

namespace paludis
{
    /**
     * Input buffer class for a process, invoked using popen(3).
     *
     * Bidirectional I/O isn't supported since we haven't needed it yet, and
     * because popen on Linux is unidirectional.
     *
     * See Josuttis' "The C++ Standard Library" Ch. 13.13 for what we're doing
     * here. The buffer code is based upon the "io/inbuf1.hpp" example in
     * section 13.13.3.
     */
    class PStreamInBuf :
        public std::streambuf,
        private InstantiationPolicy<PStreamInBuf, instantiation_method::NonCopyableTag>
    {
        private:
            const std::string _command;

            int _exit_status;

        protected:
            FILE * fd;

            static const int putback_size = std::numeric_limits<unsigned>::digits >> 3;

            static const int buffer_size = 3 * putback_size;

            char buffer[buffer_size];

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

}

#endif
