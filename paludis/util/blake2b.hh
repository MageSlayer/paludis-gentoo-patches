/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh
 * Copyright (c) 2008, 2011 David Leverton
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_BLAKE2B_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_BLAKE2B_HH 1

#include <iosfwd>
#include <string>
#include <paludis/util/attributes.hh>
#include <inttypes.h>

#include <paludis/util/blake2.h>

/** \file
 * Declarations for the Blake2b digest class.
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
     * Blake2b digest class.
     *
     * \ingroup g_digests
     */
    class PALUDIS_VISIBLE Blake2b
    {
        private:
            uint64_t H[BLAKE2B_OUTBYTES/8];

        public:
            /**
             * Constructor.
             */
            Blake2b(std::istream & stream);

            /**
             * Our checksum, as a string of hex characters.
             */
            std::string hexsum() const;
    };
}

#endif
