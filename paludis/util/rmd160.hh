/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_RMD160_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_RMD160_HH 1

#include <iosfwd>
#include <string>
#include <inttypes.h>
#include <paludis/util/attributes.hh>

/** \file
 * Declarations for the RMD160 digest class.
 *
 * \ingroup g_digests
 *
 * \section Examples
 *
 * - None at this time.
 */

namespace paludis
{
    /**
     * RMD160 digest class.
     *
     * \ingroup g_digests
     */
    class PALUDIS_VISIBLE RMD160
    {
        private:
            static const PALUDIS_HIDDEN uint8_t _r[80], _rp[80];
            static const PALUDIS_HIDDEN uint8_t _s[80], _sp[80];
            static const PALUDIS_HIDDEN uint32_t _k[5], _kp[5];

            uint32_t _h[5];
            uint64_t _size;
            bool _done_one_pad;

            void PALUDIS_HIDDEN _update(const uint8_t * const block);

            inline int PALUDIS_HIDDEN _get(std::istream & stream);

        public:
            /**
             * Constructor.
             */
            RMD160(std::istream & stream);

            /**
             * Our checksum, as a string of hex characters.
             */
            std::string hexsum() const;
    };

}

#endif

