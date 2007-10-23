/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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
#include <paludis/util/fs_entry.hh>
#include <paludis/util/private_implementation_pattern.hh>

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

            bool operator== (const DirIterator & other) const
                PALUDIS_ATTRIBUTE((warn_unused_result));
            bool operator!= (const DirIterator & other) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            ///\}
    };
}

#endif
