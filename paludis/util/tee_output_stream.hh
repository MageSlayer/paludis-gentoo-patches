/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_TEE_OUTPUT_STREAM_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_TEE_OUTPUT_STREAM_HH 1

#include <paludis/util/tee_output_stream-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/pimp.hh>
#include <ostream>

namespace paludis
{
    class PALUDIS_VISIBLE TeeOutputStreamBuf :
        public std::streambuf
    {
        private:
            Pimp<TeeOutputStreamBuf> _imp;

        protected:
            virtual int_type
            overflow(int_type c);

            virtual int sync();

            virtual std::streamsize
            xsputn(const char * s, std::streamsize num);

        public:
            TeeOutputStreamBuf();
            ~TeeOutputStreamBuf();

            void add_stream(std::ostream * const);
    };

    class PALUDIS_VISIBLE TeeOutputStreamBase
    {
        protected:
            TeeOutputStreamBuf buf;
    };

    class PALUDIS_VISIBLE TeeOutputStream :
        protected TeeOutputStreamBase,
        public std::ostream
    {
        public:
            ///\name Basic operations
            ///\{

            TeeOutputStream();

            ///\}

            void add_stream(std::ostream * const);
    };

    extern template class Pimp<TeeOutputStreamBuf>;
}

#endif
