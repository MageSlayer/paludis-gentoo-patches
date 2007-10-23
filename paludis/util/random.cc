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

#include "random.hh"
#include <time.h>

using namespace paludis;

uint32_t Random::global_seed(0xdeadbeef ^ ::time(0));

Random::Random(uint32_t seed) :
    local_seed(seed)
{
}

Random::Random() :
    local_seed(global_seed ^ ((::time(0) >> 16) | (::time(0) << 16)))
{
    global_seed += local_seed;
}

