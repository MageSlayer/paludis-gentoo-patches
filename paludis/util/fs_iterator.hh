/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_FS_ITERATOR_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_FS_ITERATOR_HH 1

#include <paludis/util/fs_iterator-fwd.hh>
#include <paludis/util/fs_path-fwd.hh>
#include <paludis/util/options-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/pimp.hh>
#include <iterator>

namespace paludis
{
    class PALUDIS_VISIBLE FSIterator
    {
        friend bool operator== (const FSIterator &, const FSIterator &);

        private:
            Pimp<FSIterator> _imp;

        public:
            explicit FSIterator();
            FSIterator(const FSPath &, const FSIteratorOptions &);

            FSIterator(const FSIterator &);

            FSIterator & operator= (const FSIterator &);

            ~FSIterator();

            typedef const FSPath & value_type;
            typedef const FSPath & reference;
            typedef const FSPath * pointer;
            typedef std::ptrdiff_t difference_type;
            typedef std::forward_iterator_tag iterator_category;

            FSIterator & operator++ ();
            FSIterator operator++ (int);

            pointer operator-> () const;
            reference operator* () const;
    };

    extern template class Pimp<FSIterator>;
}

#endif
