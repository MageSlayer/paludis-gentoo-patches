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

#include "category_name_part.hh"

using namespace paludis;

void
CategoryNamePartValidator::validate(const std::string & s)
{
    /* this gets called a lot, make it fast */

    static const std::string allowed_chars(
            "abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "0123456789-+_");

    do
    {
        if (s.empty())
            break;

        if (std::string::npos != s.find_first_not_of(allowed_chars))
            break;

        return;

    } while (false);

    Context c("When validating category name '" + s + "':");
    throw CategoryNamePartError(s);
}

CategoryNamePartError::CategoryNamePartError(const std::string & name) throw () :
    NameError(name, "category name part")
{
}

