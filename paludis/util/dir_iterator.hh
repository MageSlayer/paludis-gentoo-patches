/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#include <paludis/util/dir_iterator-fwd.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/options.hh>
#include <paludis/util/pimp.hh>
#include <iterator>

/** \file
 * Declarations for DirIterator.
 *
 * \ingroup g_fs
 *
 * \section Examples
 *
 * - None at this time.
 */

namespace paludis
{
    /**
     * Raised when a directory open fails.
     *
     * \ingroup g_fs
     * \ingroup g_exceptions
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
     * An iterator that iterates over the contents of a directory.
     *
     * \ingroup g_fs
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE DirIterator :
        private Pimp<DirIterator>
    {
        public:
            ///\name Standard library typedefs
            ///\{

            typedef FSEntry value_type;
            typedef const FSEntry & reference;
            typedef const FSEntry * pointer;
            typedef std::ptrdiff_t difference_type;
            typedef std::forward_iterator_tag iterator_category;

            ///\}

            ///\name Basic operations
            ///\{

            /**
             * Constructor, to an FSEntry which must be a directory, with an
             * option to not ignore dotfiles and an option to do inodesort.
             */
            explicit DirIterator(const FSEntry &, const DirIteratorOptions & = DirIteratorOptions());

            DirIterator(const DirIterator &);

            /**
             * Constructor, creates an end() iterator.
             */
            DirIterator();

            ~DirIterator();

            DirIterator & operator= (const DirIterator &);

            ///\}

            ///\name Dereference operators
            ///\{

            const FSEntry & operator* () const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            const FSEntry * operator-> () const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            ///\}

            ///\name Increment, decrement operators
            ///\{

            DirIterator & operator++ ();
            DirIterator operator++ (int);

            ///\}

            ///\name Comparison operators
            ///\{

            bool operator== (const DirIterator &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));
            bool operator!= (const DirIterator &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            ///\}
    };

    extern template class Pimp<DirIterator>;
}

#endif
