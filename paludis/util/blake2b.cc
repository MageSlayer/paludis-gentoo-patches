/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
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

#include <paludis/util/blake2b.hh>
#include <paludis/util/byte_swap.hh>
#include <paludis/util/digest_registry.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/exception.hh>
#include <sstream>
#include <istream>
#include <iomanip>
#include <limits>
#include <algorithm>
#include <utility>

using namespace paludis;

Blake2b::Blake2b(std::istream & s)
{
    std::fill(&H[0], &H[BLAKE2B_OUTBYTES/8], 0);

    std::streambuf * buf(s.rdbuf());

    uint8_t m[64];
    std::streamsize block_size;

    blake2b_state S;
    int err = 0;

    if ( (err = blake2b_init(&S, BLAKE2B_OUTBYTES)) < 0 ) {
        throw InternalError(PALUDIS_HERE, "Blake2B hash failed to init. Error code " + stringify(err));
    }

    do
    {
        block_size = buf->sgetn(reinterpret_cast<char *>(m), 64);

        if ( (err = blake2b_update(&S, &m[0], block_size)) < 0 ) {
            throw InternalError(PALUDIS_HERE, "Blake2B hash failed to update. Error code " + stringify(err));
        }

    } while (64 == block_size);

    if ( (err = blake2b_final(&S, &H[0], BLAKE2B_OUTBYTES)) < 0 ) {
        throw InternalError(PALUDIS_HERE, "Blake2B hash failed to finalize. Error code " + stringify(err));
    }
}

std::string
Blake2b::hexsum() const
{
    std::stringstream result;
    result << std::hex << std::right << std::setfill('0');
    for (uint64_t i : H)
        result << std::setw(16) << from_bigendian(i);
    return result.str();
}

namespace
{
    DigestRegistry::Registration<Blake2b> registration("BLAKE2B");
}
