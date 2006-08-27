/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <functional>
#include <paludis/util/fs_entry.hh>
#include <string>

/** \file
 * Declarations for the IsFileWithExtension class.
 *
 * \ingroup grpfilesystem
 */

namespace paludis
{
    /**
     * The IsFileWithExtension class is a functor that determines whether an
     * FSEntry instance is a file with a given extension and (optionally) a
     * given filename prefix.
     *
     * \ingroup grpfilesystem
     */
    class IsFileWithExtension :
        public std::unary_function<bool, FSEntry>
    {
        private:
            const std::string _prefix;
            const std::string _ext;

        public:
            /**
             * Constructor.
             */
            IsFileWithExtension(const std::string & ext);

            /**
             * Constructor.
             */
            IsFileWithExtension(const std::string & prefix, const std::string & ext);

            /**
             * Operator.
             */
            bool operator() (const FSEntry & f) const;
    };
}

#endif
