/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2013 Wouter van Kesteren <woutershep@gmail.com>
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

#ifndef PALUDIS_GUARD_PALUDIS_STREAM_HOLDER_HH
#define PALUDIS_GUARD_PALUDIS_STREAM_HOLDER_HH 1

#include <paludis/util/attributes.hh>
#include <utility>
#include <ostream>
#include <ios>

namespace paludis
{
    /**
     * Holder for a stream so the destructor can throw.
     */
    template <typename Stream_>
    class PALUDIS_VISIBLE StreamHolder
    {
        private:
            Stream_ s;
            typedef std::basic_ostream<typename Stream_::char_type, typename Stream_::traits_type> ostream_type;
            typedef std::basic_ios<typename Stream_::char_type, typename Stream_::traits_type> ios_type;

        public:
            ///\name Basic operations
            ///\{

            template<typename... Args_>
            StreamHolder(Args_ && ... args) : s(std::forward<Args_>(args)...)
            {
            }

            ///\}

            ///\name Formatted output
            ///\{

            template <typename T_>
            StreamHolder<Stream_>& operator<<(T_ && rhs) 
            {
                s << std::forward<T_>(rhs);
                return *this;
            }

            /**
             * Overload for stream manipulators (std::endl, std::flush, etc.)
             *
             * see 27.7.3.6.3
             */
            ostream_type& operator<<(ostream_type& (*rhs)(ostream_type&))
            {
                return rhs(s);
            }

            /**
             * Overload for stream manipulators (std::endl, std::flush, etc.)
             *
             * see 27.7.3.6.3
             */
            ostream_type& operator<<(ios_type& (*rhs)(ios_type&))
            {
                rhs(s);
                return s;
            }

            /**
             * Overload for stream manipulators (std::endl, std::flush, etc.)
             *
             * see 27.7.3.6.3
             */
            ostream_type& operator<<(std::ios_base& (*rhs)(std::ios_base&))
            {
                rhs(s);
                return s;
            }

            ///\}

            ///\name Conversions
            ///\{

            bool operator!() const
            {
                return !s;
            }

            operator bool() const
            {
                return s;
            }

            operator Stream_&()
            {
                return s;
            }

            operator Stream_*()
            {
                return &s;
            }

            ///\}
    };
}

#endif
