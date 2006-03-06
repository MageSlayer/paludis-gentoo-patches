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

#include <paludis/qualified_package_name.hh>
#include <paludis/util/stringify.hh>

using namespace paludis;

QualifiedPackageNameError::QualifiedPackageNameError(const std::string & s) throw () :
    NameError(s, "qualified package name")
{
}

std::ostream &
paludis::operator<< (std::ostream & s, const QualifiedPackageName & q)
{
    s << q.get<qpn_category>() << "/" << q.get<qpn_package>();
    return s;
}

MakeSmartRecord<QualifiedPackageNameTag>::Type
QualifiedPackageName::_make_parent(
        const std::string & s)
{
    Context c("When splitting out category and package names from '" + s + "':");

    std::string::size_type p(s.find('/'));
    if (std::string::npos == p)
        throw QualifiedPackageNameError(s);

    return MakeSmartRecord<QualifiedPackageNameTag>::Type(
            CategoryNamePart(s.substr(0, p)),
            PackageNamePart(s.substr(p + 1)));
}

