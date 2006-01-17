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

#include "qualified_package_name.hh"
#include "qualified_package_name_error.hh"
#include "stringify.hh"

using namespace paludis;

std::string get_category_name(const std::string & s)
{
    Context c("When splitting out category name from '" + s + "':");

    std::string::size_type p(s.find('/'));
    if (std::string::npos == p)
        throw QualifiedPackageNameError(s);
    return s.substr(0, p);
}

std::string get_package_name(const std::string & s)
{
    Context c("When splitting out package name from '" + s + "':");

    std::string::size_type p(s.find('/'));
    if (std::string::npos == p)
        throw QualifiedPackageNameError(s);
    return s.substr(p + 1);
}

std::ostream &
paludis::operator<< (std::ostream & s, const QualifiedPackageName & q)
{
    s << q.get<qpn_category>() << "/" << q.get<qpn_package>();
    return s;
}

QualifiedPackageName
paludis::make_qualified_package_name(const std::string & s)
{
    return QualifiedPackageName(CategoryNamePart(get_category_name(s)),
            PackageNamePart(get_package_name(s)));
}

