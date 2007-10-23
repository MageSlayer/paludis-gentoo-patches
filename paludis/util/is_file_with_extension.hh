/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#ifndef PALUDIS_GUARD_PALUDIS_IS_FILE_WITH_EXTENSION_HH
#define PALUDIS_GUARD_PALUDIS_IS_FILE_WITH_EXTENSION_HH 1

#include <paludis/util/fs_entry.hh>
#include <paludis/util/options.hh>
#include <functional>
#include <string>

/** \file
 * Declarations for the is_file_with_extension function.
 *
 * \ingroup g_fs
 *
 * \section Examples
 *
 * - None at this time.
 */

namespace paludis
{

#include <paludis/util/is_file_with_extension-se.hh>

    /**
     * Options for is_file_with_prefix_extension() and is_file_with_extension().
     *
     * \see IsFileWithOption
     * \see is_file_with_prefix_extension
     * \see is_file_with_extension
     * \ingroup g_fs
     */
    typedef Options<IsFileWithOption> IsFileWithOptions;

    /**
     * Return whether an FSEntry is a file with a given extension.
     *
     * \see is_file_with_prefix_extension()
     *
     * \ingroup g_fs
     */
    bool is_file_with_extension(const FSEntry &, const std::string &, const IsFileWithOptions &) PALUDIS_VISIBLE;

    /**
     * Return whether an FSEntry is a file with a given prefix and a given
     * extension prefix.
     *
     * \see is_file_with_extension()
     *
     * \ingroup g_fs
     */
    bool is_file_with_prefix_extension(const FSEntry &, const std::string &, const std::string &, const IsFileWithOptions &) PALUDIS_VISIBLE;

    /**
     * The IsFileWithExtension class is a functor that determines whether an
     * FSEntry instance is a file with a given extension and (optionally) a
     * given filename prefix.
     *
     * \ingroup g_fs
     * \deprecated Use is_file_with_extension and tr1::bind.
     */
#ifdef DOXYGEN
    class IsFileWithExtension :
#else
    class PALUDIS_VISIBLE PALUDIS_ATTRIBUTE((deprecated)) IsFileWithExtension :
#endif
        public std::unary_function<bool, FSEntry>
    {
        private:
            const std::string _prefix;
            const std::string _ext;

        public:
            /**
             * Constructor.
             */
            IsFileWithExtension(const std::string & ext) PALUDIS_ATTRIBUTE((deprecated));

            /**
             * Constructor.
             */
            IsFileWithExtension(const std::string & prefix, const std::string & ext) PALUDIS_ATTRIBUTE((deprecated));

            /**
             * Operator.
             */
            bool operator() (const FSEntry & f) const
                PALUDIS_ATTRIBUTE((warn_unused_result));
    };
}

#endif
