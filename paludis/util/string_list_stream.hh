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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_STRING_LIST_STREAM_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_STRING_LIST_STREAM_HH 1

#include <paludis/util/string_list_stream-fwd.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/attributes.hh>
#include <istream>
#include <ostream>

namespace paludis
{
    class PALUDIS_VISIBLE StringListStreamBuf :
        private PrivateImplementationPattern<StringListStreamBuf>,
        public std::streambuf
    {
        protected:
            virtual int_type underflow();

            virtual int_type overflow(int_type c);
            virtual std::streamsize xsputn(const char * s, std::streamsize num);

        public:
            StringListStreamBuf();
            ~StringListStreamBuf();

            void nothing_more_to_write();
    };

    class PALUDIS_VISIBLE StringListStreamBase
    {
        protected:
            StringListStreamBuf buf;

        public:
            StringListStreamBase();
            ~StringListStreamBase();
    };

    class PALUDIS_VISIBLE StringListStream :
        protected StringListStreamBase,
        public std::istream,
        public std::ostream
    {
        public:
            StringListStream();
            ~StringListStream();

            void nothing_more_to_write();
    };

    extern template class PrivateImplementationPattern<StringListStreamBuf>;
}

#endif
