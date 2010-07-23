/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Fernando J. Pereda
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

#ifndef PALUDIS_GUARD_PALUDIS_DAMERAU_LEVENSHTEIN_HH
#define PALUDIS_GUARD_PALUDIS_DAMERAU_LEVENSHTEIN_HH 1

#include <paludis/util/pimp.hh>
#include <paludis/util/attributes.hh>
#include <string>

/** \file
 * Declarations for paludis::DamerauLevenshtein
 *
 * \ingroup g_utils
 */

namespace paludis
{
    /**
     * Object to calculate Damerau-Levenshtein distances between two strings.
     *
     * \ingroup g_utils
     */
    class PALUDIS_VISIBLE DamerauLevenshtein :
        private Pimp<DamerauLevenshtein>
    {
        public:
            ///\name Basic Operations
            ///\{

            DamerauLevenshtein(const std::string & name);
            ~DamerauLevenshtein();

            /**
             * Compute the Damerau-Levenshtein to this candidate.
             */
            unsigned distance_with(const std::string & candidate) const;

            ///\}
    };

    extern template class Pimp<DamerauLevenshtein>;
}

#endif
