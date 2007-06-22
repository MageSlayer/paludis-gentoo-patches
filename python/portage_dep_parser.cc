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
#include <paludis/eapi.hh>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

void PALUDIS_VISIBLE expose_portage_dep_parser()
{
    /**
     * Exceptions
     */
    ExceptionRegister::get_instance()->add_exception<DepStringError>
        ("DepStringError", "BaseException",
         "A DepStringError descendent is thrown if an invalid depend string is encountered.");
    ExceptionRegister::get_instance()->add_exception<DepStringLexError>
        ("DepStringLexError", "DepStringError",
         "A DepStringLexError is thrown if a lex-level error is encountered when parsing a dependency string.");
    ExceptionRegister::get_instance()->add_exception<DepStringParseError>
        ("DepStringParseError", "DepStringError",
         "Thrown if an error is encountered when parsing a dependency string.");
    ExceptionRegister::get_instance()->add_exception<DepStringNestingError>
        ("DepStringNestingError", "DepStringParseError",
         "Thrown if a dependency string does not have properly balanced parentheses.");

    /**
     * PortageDepParser
     */
    bp::class_<PortageDepParser, boost::noncopyable>
        (
         "PortageDepParser",
         "The PortageDepParser converts string representations "
         "of a dependency specification into a DepSpec instance.",
         bp::no_init
        )
        .def("parse_depend", &PortageDepParser::parse_depend,
                "parse_depend(string, EAPI) -> CompositeDepSpec\n"
                "Parse a dependency heirarchy."
            )
        .staticmethod("parse_depend")

        .def("parse_provide", &PortageDepParser::parse_provide,
                "parse_provide(string, EAPI) -> CompositeDepSpec\n"
                "Parse a provide heirarchy."
            )
        .staticmethod("parse_provide")

        .def("parse_restrict", &PortageDepParser::parse_restrict,
                "parse_restrict(string, EAPI) -> CompositeDepSpec\n"
                "Parse a restrict."
            )
        .staticmethod("parse_restrict")

        .def("parse_uri", &PortageDepParser::parse_uri,
                "parse_uri(string, EAPI) -> CompositeDepSpec\n"
                "Parse a uri heirarchy."
            )
        .staticmethod("parse_uri")

        .def("parse_license", &PortageDepParser::parse_license,
                "parse_license(string, EAPI) -> CompositeDepSpec\n"
                "Parse a license heirarchy."
            )
        .staticmethod("parse_license")
        ;
}
