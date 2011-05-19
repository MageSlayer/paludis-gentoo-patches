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
#include <python/iterable.hh>

#include <paludis/version_requirements.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/make_named_values.hh>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

namespace
{
    VersionRequirement * make_version_requirement(const VersionOperator & op, const VersionSpec & spec)
    {
        return new VersionRequirement(make_named_values<VersionRequirement>(
                    n::version_operator() = op,
                    n::version_spec() = spec
                    ));
    }
}

void expose_version_requirements()
{
    /**
     * Enums
     */
    enum_auto("VersionRequirementsMode", last_vr,
            "Whether our version requirements are an 'and' or an 'or' set.");

    /**
     * VersionRequirement
     */
    bp::class_<VersionRequirement>
        (
         "VersionRequirement",
         bp::no_init
        )

        .def("__init__",
                bp::make_constructor(&make_version_requirement),
                "__init__(VersionOperator, VersionSpec)"
            )

        .add_property("version_operator",
                &named_values_getter<VersionRequirement, n::version_operator, VersionOperator, &VersionRequirement::version_operator>,
                &named_values_setter<VersionRequirement, n::version_operator, VersionOperator, &VersionRequirement::version_operator>,
                "[rw] VersionOperator"
                )

        .add_property("version_spec",
                &named_values_getter<VersionRequirement, n::version_spec, VersionSpec, &VersionRequirement::version_spec>,
                &named_values_setter<VersionRequirement, n::version_spec, VersionSpec, &VersionRequirement::version_spec>,
                "[rw] VersionSpec"
                )
        ;

    /**
     * VersionRequirements
     */
    class_iterable<VersionRequirements>
        (
         "VersionRequirements",
         "Iterable collection of VersionRequirement instances, usually for a PackageDepSpec."
        );
}
