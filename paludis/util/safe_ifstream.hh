/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_SAFE_IFSTREAM_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_SAFE_IFSTREAM_HH 1

#include <paludis/util/attributes.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/fs_path-fwd.hh>
#include <istream>

/** \file
 * Declarations for SafeIFStream.
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
     * Input stream buffer class that can be opened via an FD, and that doesn't
     * do retarded things when given a non-file.
     *
     * See \ref TCppSL Ch. 13.13 for what we're doing here. The buffer code is
     * based upon the "io/inbuf1.hpp" example in section 13.13.3.
     *
     * \ingroup g_fs
     * \since 0.34.3
     */
    class PALUDIS_VISIBLE SafeIFStreamBuf :
        public std::streambuf
    {
        protected:
            static const int lookbehind_size = 16;
            static const int buffer_size = 512 + lookbehind_size;
            char buffer[buffer_size];

            int_type underflow() override;
            pos_type seekoff(off_type, std::ios_base::seekdir, std::ios_base::openmode) override;
            pos_type seekpos(pos_type, std::ios_base::openmode) override;

        public:
            ///\name Basic operations
            ///\{

            SafeIFStreamBuf(const int f);

            ///\}

            /// Our file descriptor.
            int fd;
    };

    /**
     * Member from base initialisation for SafeIFStream.
     *
     * \ingroup g_fs
     * \since 0.34.3
     */
    class PALUDIS_VISIBLE SafeIFStreamBase
    {
        protected:
            /// Our buffer.
            SafeIFStreamBuf buf;

        public:
            ///\name Basic operations
            ///\{

            SafeIFStreamBase(const int fd);

            ///\}
    };

    /**
     * Input stream buffer class that can be opened via an FD, and that doesn't
     * do retarded things when given a non-file.
     *
     * \ingroup g_fs
     * \since 0.34.3
     */
    class PALUDIS_VISIBLE SafeIFStream :
        protected SafeIFStreamBase,
        public std::istream
    {
        private:
            const bool _close;

        public:
            ///\name Basic operations
            ///\{

            explicit SafeIFStream(const int fd);
            explicit SafeIFStream(const FSPath &);
            ~SafeIFStream() override;

            ///\}
    };

    /**
     * Thrown by SafeIFStream if an error occurs.
     *
     * \ingroup g_fs
     * \since 0.34.3
     */
    class PALUDIS_VISIBLE SafeIFStreamError :
        public Exception
    {
        public:
            SafeIFStreamError(const std::string &) noexcept;
    };
}

#endif
