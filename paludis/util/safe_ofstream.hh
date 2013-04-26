/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_SAFE_OFSTREAM_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_SAFE_OFSTREAM_HH 1

#include <paludis/util/attributes.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/fs_path-fwd.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/stream_holder.hh>
#include <ostream>

/** \file
 * Declarations for SafeOFStream.
 *
 * \ingroup g_fs
 *
 * \section Examples
 *
 * - None at this time.
 */

namespace paludis
{
    /**
     * Output stream buffer class that can be opened via an FD, and that doesn't
     * do retarded things when given a non-file.
     *
     * See \ref TCppSL Ch. 13.13 for what we're doing here. The buffer code is
     * based upon the "io/outbuf2.hpp" example in section 13.13.3.
     *
     * \ingroup g_fs
     * \since 0.34.3
     */
    class PALUDIS_VISIBLE SafeOFStreamBuf :
        public std::streambuf
    {
        private:
            Pimp<SafeOFStreamBuf> _imp;

        protected:
            virtual int_type
            overflow(int_type c);

            virtual std::streamsize
            xsputn(const char * s, std::streamsize num);

        public:
            ///\name Basic operations
            ///\{

            SafeOFStreamBuf(const int f, const bool buffer);
            ~SafeOFStreamBuf();

            ///\}

            void write_buffered();

            /// Our file descriptor.
            int fd;
    };

    /**
     * Member from base initialisation for SafeOFStream.
     *
     * \ingroup g_fs
     * \since 0.34.3
     */
    class PALUDIS_VISIBLE SafeOFStreamBase
    {
        protected:
            /// Our buffer.
            SafeOFStreamBuf buf;

        public:
            ///\name Basic operations
            ///\{

            SafeOFStreamBase(const int fd, const bool buffer);

            ///\}
    };

    /**
     * Output stream buffer class that can be opened via an FD, and that doesn't
     * do retarded things when given a non-file.
     *
     * \ingroup g_fs
     * \since 0.34.3
     */
    class PALUDIS_VISIBLE SafeOFStream :
        protected SafeOFStreamBase,
        public StreamHolder<std::ostream>
    {
        private:
            const bool _close;

        public:
            ///\name Basic operations
            ///\{

            SafeOFStream(const int fd, const bool buffer);
            SafeOFStream(const FSPath &, const int open_flags, const bool buffer);
            ~SafeOFStream();

            ///\}
    };

    /**
     * Thrown by SafeOFStream if an error occurs.
     *
     * \ingroup g_fs
     * \since 0.34.3
     */
    class PALUDIS_VISIBLE SafeOFStreamError :
        public Exception
    {
        public:
            SafeOFStreamError(const std::string &) throw ();
    };

    extern template class Pimp<SafeOFStreamBuf>;
}

#endif
