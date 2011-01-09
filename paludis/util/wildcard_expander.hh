/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 David Leverton
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_WILDCARD_EXPANDER_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_WILDCARD_EXPANDER_HH

#include <paludis/util/attributes.hh>
#include <paludis/util/fs_path.hh>
#include <paludis/util/fs_error.hh>
#include <paludis/util/operators.hh>
#include <paludis/util/pimp.hh>

#include <iterator>

namespace paludis
{
    class PALUDIS_VISIBLE WildcardExpansionError :
        public FSError
    {
        public:
            WildcardExpansionError(const std::string & message) throw ();
    };

    class PALUDIS_VISIBLE WildcardExpander :
        public std::iterator<std::forward_iterator_tag, const FSPath>,
        public equality_operators::HasEqualityOperators
    {
        private:
            Pimp<WildcardExpander> _imp;

        public:
            WildcardExpander(const std::string &, const FSPath & = FSPath("/"));
            WildcardExpander();
            WildcardExpander(const WildcardExpander &);

            ~WildcardExpander();

            WildcardExpander & operator= (const WildcardExpander &);

            const FSPath & operator* () const PALUDIS_ATTRIBUTE((warn_unused_result));
            const FSPath * operator-> () const PALUDIS_ATTRIBUTE((warn_unused_result));

            WildcardExpander & operator++ ();
            WildcardExpander operator++ (int);

            bool operator== (const WildcardExpander &) const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    extern template class Pimp<WildcardExpander>;
}

#endif
