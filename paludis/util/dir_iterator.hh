/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <iterator>
#include <paludis/util/counted_ptr.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/private_implementation_pattern.hh>

/** \file
 * Declarations for paludis::DirIterator.
 *
 * \ingroup grpfilesystem
 */

namespace paludis
{
    /**
     * Raised when a directory open fails.
     *
     * \ingroup grpfilesystem
     * \ingroup grpexceptions
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE DirOpenError :
        public FSError
    {
        public:
            ///\name Basic operations
            ///\{

            DirOpenError(const FSEntry & location, const int errno_value) throw ();

            ///\}
    };

    /**
     * An iterator that iterates over the contents of a directory. At present,
     * we read in all the entries at creation time and maintain a CountedPtr
     * to an ordered set of FSEntry instances. This may change at some point,
     * if it turns out that it's quicker to use opendir and seekdir for each
     * instance.
     *
     * \ingroup grpfilesystem
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE DirIterator :
        public std::iterator<std::forward_iterator_tag, FSEntry>,
        private PrivateImplementationPattern<DirIterator>
    {
        public:
            ///\name Basic operations
            ///\{

            /**
             * Constructor, to an FSEntry which must be a directory, with an
             * option to not ignore dotfiles.
             */
            explicit DirIterator(const FSEntry & base, bool ignore_dotfiles = true);

            DirIterator(const DirIterator & other);

            /**
             * Constructor, creates an end() iterator.
             */
            DirIterator();

            ~DirIterator();

            const DirIterator & operator= (const DirIterator & other);

            ///\}

            ///\name Dereference operators
            ///\{

            const FSEntry & operator* () const;

            const FSEntry * operator-> () const;

            ///\}

            ///\name Increment, decrement operators
            ///\{

            DirIterator & operator++ ();
            DirIterator operator++ (int);

            ///\}

            ///\name Comparison operators
            ///\{

            bool operator== (const DirIterator & other) const;
            bool operator!= (const DirIterator & other) const;

            ///\}
    };
}

#endif
