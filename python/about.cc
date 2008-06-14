/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Piotr Jaroszyński
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

#include <python/paludis_python.hh>

#include <paludis/about.hh>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

void expose_about()
{
    bp::scope s;
    s.attr("VERSION_MAJOR") = PALUDIS_VERSION_MAJOR;
    s.attr("VERSION_MINOR") = PALUDIS_VERSION_MINOR;
    s.attr("VERSION_MICRO") = PALUDIS_VERSION_MICRO;
    s.attr("VERSION") = stringify(PALUDIS_VERSION_MAJOR) + "."
        + stringify(PALUDIS_VERSION_MINOR) + "." + stringify(PALUDIS_VERSION_MICRO);
    s.attr("VERSION_SUFFIX") = stringify(PALUDIS_VERSION_SUFFIX);
    s.attr("GIT_HEAD") = stringify(PALUDIS_GIT_HEAD);
}
