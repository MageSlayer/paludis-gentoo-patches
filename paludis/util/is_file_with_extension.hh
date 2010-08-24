/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2010 Ciaran McCreesh
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

#include <paludis/util/fs_path-fwd.hh>
#include <paludis/util/options-fwd.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/stringify.hh>
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
     * Return whether an FSPath is a file with a given extension.
     *
     * \see is_file_with_prefix_extension()
     *
     * \ingroup g_fs
     */
    bool is_file_with_extension(const FSPath &, const std::string &, const IsFileWithOptions &) PALUDIS_VISIBLE;

    /**
     * Return whether an FSPath is a file with a given prefix and a given
     * extension prefix.
     *
     * \see is_file_with_extension()
     *
     * \ingroup g_fs
     */
    bool is_file_with_prefix_extension(const FSPath &, const std::string &, const std::string &, const IsFileWithOptions &) PALUDIS_VISIBLE;
}

#endif
