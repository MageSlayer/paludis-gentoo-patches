/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_DIR_ITERATOR_HH
#define PALUDIS_GUARD_PALUDIS_DIR_ITERATOR_HH 1

#include <paludis/util/fs_entry.hh>
#include <paludis/util/counted_ptr.hh>
#include <iterator>
#include <set>

/** \file
 * Declarations for paludis::DirIterator.
 *
 * \ingroup Filesystem
 * \ingroup Exception
 */

namespace paludis
{
    /**
     * Raised when a directory open fails.
     *
     * \ingroup Exception
     */
    class DirOpenError : public FSError
    {
        public:
            /**
             * Constructor.
             */
            DirOpenError(const FSEntry & location, const int errno_value) throw ();
    };

    /**
     * An iterator that iterates over the contents of a directory. At present,
     * we read in all the entries at creation time and maintain a CountedPtr
     * to an ordered set of FSEntry instances. This may change at some point,
     * if it turns out that it's quicker to use opendir and seekdir for each
     * instance.
     *
     * \ingroup Filesystem
     */
    class DirIterator : public std::iterator<std::forward_iterator_tag, FSEntry>
    {
        private:
            FSEntry _base;
            CountedPtr<std::set<FSEntry>, count_policy::ExternalCountTag> _items;
            std::set<FSEntry>::iterator _iter;

        public:
            /**
             * Constructor, to an FSEntry which must be a directory.
             */
            explicit DirIterator(const FSEntry & base);

            /**
             * Copy constructor.
             */
            DirIterator(const DirIterator & other);

            /**
             * Constructor, creates an end() iterator.
             */
            DirIterator();

            /**
             * Destructor.
             */
            ~DirIterator();

            /**
             * Assign.
             */
            const DirIterator & operator= (const DirIterator & other);

            /**
             * Dereference.
             */
            const FSEntry & operator* () const;

            /**
             * Dereference.
             */
            const FSEntry * operator-> () const;

            /**
             * Increment.
             */
            DirIterator & operator++ ();

            /**
             * Increment.
             */
            DirIterator operator++ (int);

            /**
             * Compare.
             */
            bool operator== (const DirIterator & other) const;

            /**
             * Inverse compare.
             */
            bool operator!= (const DirIterator & other) const;
    };
}

#endif
