/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Piotr Jaroszyński <peper@gentoo.org>
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

#include <paludis_python.hh>

#include <paludis/version_requirements.hh>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

void PALUDIS_VISIBLE expose_version_requirements()
{
    bp::class_<VersionRequirement>
        vr("VersionRequirement",
                bp::init<const VersionOperator &, const VersionSpec &>(
                    "__init__(VersionOperator, VersionSpec)"
                    )
          );
    vr.def_readwrite("version_operator", &VersionRequirement::version_operator,
            "[rw] VersionOperator"
            );
    vr.def_readwrite("version_spec", &VersionRequirement::version_spec,
            "[rw] VersionSpec"
            );
    vr.def("__eq__", &VersionRequirement::operator==);
    vr.def("__neq__", &VersionRequirement::operator!=);

    enum_auto("VersionRequirementsMode", last_vr);

    class_collection<VersionRequirements>
        vrc("VersionRequirements",
                "Iterable collection of VersionRequirement instances, usually for a PackageDepSpec."
           );
}
