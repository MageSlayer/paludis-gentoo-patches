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

#include "package_name_part.hh"

using namespace paludis;

PackageNamePartError::PackageNamePartError(const std::string & name) throw () :
    NameError(name, "package name part")
{
}

void
PackageNamePartValidator::validate(const std::string & s)
{
    /* this gets called a lot, make it fast */

    static const std::string allowed_chars(
            "abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "0123456789-+_");

    static const std::string number_chars(
            "0123456789");

    if (s.empty() || '-' == s.at(0))
    {
        Context c("When validating package name part '" + s + "':");
        throw PackageNamePartError(s);
    }

    /* we don't allow - followed by only numbers, because it could be
     * a version spec. */
    for (std::string::size_type p(0) ; p < s.length() ; ++p)
    {
        if (std::string::npos == allowed_chars.find(s[p]))
        {
            Context c("When validating package name part '" + s + "':");
            throw PackageNamePartError(s);
        }

        if ('-' != s[p])
            continue;
        if (++p >= s.length())
            break;
        if ((std::string::npos != number_chars.find(s[p]) &&
                std::string::npos == s.find_first_not_of(number_chars, p)) ||
                (std::string::npos == allowed_chars.find(s[p])))
        {
            Context c("When validating package name part '" + s + "':");
            throw PackageNamePartError(s);
        }
    }
}

