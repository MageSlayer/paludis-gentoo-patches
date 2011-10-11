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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_WHIRLPOOL_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_WHIRLPOOL_HH 1

#include <iosfwd>
#include <string>
#include <paludis/util/attributes.hh>
#include <inttypes.h>

/** \file
 * Declarations for the Whirlpool digest class.
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
     * Whirlpool digest class.
     *
     * \ingroup g_digests
     */
    class PALUDIS_VISIBLE Whirlpool
    {
        private:
            uint64_t H[8];

            void PALUDIS_HIDDEN process_block(const uint64_t *);

        public:
            /**
             * Constructor.
             */
            Whirlpool(std::istream & stream);

            /**
             * Our checksum, as a string of hex characters.
             */
            std::string hexsum() const;
    };
}

#endif
