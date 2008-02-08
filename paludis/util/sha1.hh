/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh
 * Copyright (c) 2008 David Leverton
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_SHA1_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_SHA1_HH 1

#include <iosfwd>
#include <string>
#include <inttypes.h>
#include <paludis/util/attributes.hh>

/** \file
 * Declarations for the SHA-1 digest class.
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
     * SHA-1 digest class.
     *
     * \ingroup g_digests
     */
    class PALUDIS_VISIBLE SHA1
    {
        private:
            uint32_t h0, h1, h2, h3, h4;

            void PALUDIS_HIDDEN process_block(uint32_t *);

        public:
            /**
             * Constructor.
             */
            SHA1(std::istream & stream);

            /**
             * Our checksum, as a string of hex characters.
             */
            std::string hexsum() const;
    };
}

#endif
