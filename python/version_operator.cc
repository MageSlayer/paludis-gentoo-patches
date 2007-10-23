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
#include <python/exception.hh>

#include <paludis/version_operator.hh>
#include <paludis/version_spec.hh>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

struct VersionOperatorWrapper
{
    // Compare two VersionSpecs
    static bool
    compare_vs(const VersionOperator & self, const VersionSpec & v1, const VersionSpec & v2)
    {
        return self.as_version_spec_comparator()(v1, v2);
    }
};

void expose_version_operator()
{
    /**
     * Exceptions
     */
    ExceptionRegister::get_instance()->add_exception<BadVersionOperatorError>
        ("BadVersionOperatorError", "BaseException",
         "Thrown if a bad version operator is encountered.");

    /**
     * Enums
     */
    enum_auto("VersionOperatorValue", last_vo,
            "Represents an operator attached to a VersionSpec.");

    /**
     * VersionOperator
     */
    bp::implicitly_convertible<std::string, VersionOperator>();
    bp::implicitly_convertible<VersionOperatorValue, VersionOperator>();
    bp::class_<VersionOperator>
        (
         "VersionOperator",
         "An operator attached to a VersionSpec, validated.",
         bp::init<const VersionOperatorValue>("__init__(VersionOperatorValue)")
        )
        .def(bp::init<const std::string &>("__init__(string)"))

        .add_property("value", &VersionOperator::value,
                "[ro] VersionOperatorValue"
                )

        .def("compare", &VersionOperatorWrapper::compare_vs,
                "compare(VersionSpec, VersionSpec) -> bool\n"
                "Compare two VersionSpecs with this operator."
            )

        .def(bp::self_ns::str(bp::self))
        ;
}
