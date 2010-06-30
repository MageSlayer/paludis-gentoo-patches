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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_BUFFER_OUTPUT_STREAM_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_BUFFER_OUTPUT_STREAM_HH 1

#include <paludis/util/buffer_output_stream-fwd.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <ostream>

namespace paludis
{
    class PALUDIS_VISIBLE BufferOutputStreamBuf :
        private PrivateImplementationPattern<BufferOutputStreamBuf>,
        public std::streambuf
    {
        protected:
            virtual int_type overflow(int_type c);
            virtual std::streamsize xsputn(const char * s, std::streamsize num);

        public:
            BufferOutputStreamBuf();
            ~BufferOutputStreamBuf();

            void unbuffer(std::ostream &);
            bool anything_to_unbuffer() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE BufferOutputStreamBase
    {
        protected:
            BufferOutputStreamBuf buf;

        public:
            BufferOutputStreamBase();
            ~BufferOutputStreamBase();
    };

    class PALUDIS_VISIBLE BufferOutputStream :
        protected BufferOutputStreamBase,
        public std::ostream
    {
        public:
            BufferOutputStream();
            ~BufferOutputStream();

            void unbuffer(std::ostream &);
            bool anything_to_unbuffer() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    extern template class PrivateImplementationPattern<BufferOutputStreamBuf>;
}

#endif
