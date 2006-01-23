/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@gentoo.org>
 *
 * This file is part of the Paludis package manager. Paludis is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
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

#ifndef PALUDIS_GUARD_PALUDIS_GETENV_HH
#define PALUDIS_GUARD_PALUDIS_GETENV_HH 1

#include <paludis/exception.hh>
#include <string>

namespace paludis
{
    /**
     * Thrown if getenv_or_error fails.
     *
     * \ingroup Exception
     */
    class GetenvError : public Exception
    {
        public:
            /**
             * Constructor.
             */
            GetenvError(const std::string & key) throw ();
    };

    /**
     * Fetch the value of environment variable key, or def if the variable is
     * not defined.
     */
    std::string getenv_with_default(const std::string & key, const std::string & def);

    /**
     * Fetch the value of environment variable key, or throw a GetenvError if
     * the variable is not defined.
     */
    std::string getenv_or_error(const std::string & key);
}

#endif
