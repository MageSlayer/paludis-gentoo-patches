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

#include <paludis/portage_dep_parser.hh>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

void expose_portage_dep_parser()
{
    static register_exception<DepStringParseError>
        DepStringParseError("DepStringParseError");
    static register_exception<DepStringNestingError>
        DepStringNestingError("DepStringNestingError");

    bp::class_<PortageDepParser, boost::noncopyable>
        pdp("PortageDepParser",
                "The PortageDepParser converts string representations "
                "of a dependency specification into a DepSpec instance.",
                bp::no_init
           );
    pdp.def("parse", &PortageDepParser::parse,
            "parse(string, Policy) -> CompositeDepSpec\n"
            "Parse a given dependency string, and return an appropriate DepSpec tree."
           );
    pdp.staticmethod("parse");
    pdp.def("parse_depend", &PortageDepParser::parse_depend,
            "parse_depend(string, PackageDepSpecParseMode) -> CompositeDepSpec\n"
            "Convenience wrapper for parse for depend strings, for VersionMetadata."
           );
    pdp.staticmethod("parse_depend");
    pdp.def("parse_license", &PortageDepParser::parse_license,
            "parse_license(string) -> CompositeDepSpec\n"
            "Convenience wrapper for parse for license strings, for VersionMetadata."
           );
    pdp.staticmethod("parse_license");

    bp::scope s = pdp;
    bp::class_<PortageDepParser::Policy>
        pdpp("Policy",
                "The Policy class describes how to convert a string representation"
                "of a dependency specification into a DepSpec instance.",
                bp::no_init
            );
    pdpp.def("text_is_text_dep_spec", &PortageDepParser::Policy::text_is_text_dep_spec,
            "text_is_text_dep_spec(permit_any_deps_boolean) -> Policy\n"
            "Returns a new policy for a PlainTextDepSpec."
            );
    pdpp.staticmethod("text_is_text_dep_spec");
    pdpp.def("text_is_package_dep_spec", &PortageDepParser::Policy::text_is_package_dep_spec,
            "text_is_package_dep_spec(permit_any_deps_boolean, PackageDepSpecParseMode) -> Policy\n"
            "Returns a new policy for a PackageDepSpec."
            );
    pdpp.staticmethod("text_is_package_dep_spec");
    pdpp.add_property("permit_any_deps", &PortageDepParser::Policy::permit_any_deps);
}
