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
#include <dep_spec.hh>

#include <paludis/portage_dep_parser.hh>
#include <paludis/eapi.hh>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

void PALUDIS_VISIBLE expose_portage_dep_parser()
{
    ExceptionRegister::get_instance()->add_exception<DepStringError>
        ("DepStringError", "BaseException");
    ExceptionRegister::get_instance()->add_exception<DepStringLexError>
        ("DepStringLexError", "DepStrinError");
    ExceptionRegister::get_instance()->add_exception<DepStringParseError>
        ("DepStringParseError", "DepStringError");
    ExceptionRegister::get_instance()->add_exception<DepStringNestingError>
        ("DepStringNestingError", "DepStringParseError");

    bp::class_<PortageDepParser, boost::noncopyable>
        pdp("PortageDepParser",
                "The PortageDepParser converts string representations "
                "of a dependency specification into a DepSpec instance.",
                bp::no_init
           );
    pdp.def("parse_depend", &PortageDepParser::parse_depend,
            "parse_depend(string, EAPI) -> CompositeDepSpec\n"
            "Parse a dependency heirarchy."
           );
    pdp.staticmethod("parse_depend");
    pdp.def("parse_provide", &PortageDepParser::parse_provide,
            "parse_provide(string, EAPI) -> CompositeDepSpec\n"
            "Parse a provide heirarchy."
           );
    pdp.staticmethod("parse_provide");
    pdp.def("parse_restrict", &PortageDepParser::parse_restrict,
            "parse_restrict(string, EAPI) -> CompositeDepSpec\n"
            "Parse a restrict."
           );
    pdp.staticmethod("parse_restrict");
    pdp.def("parse_uri", &PortageDepParser::parse_uri,
            "parse_uri(string, EAPI) -> CompositeDepSpec\n"
            "Parse a uri heirarchy."
           );
    pdp.staticmethod("parse_uri");
    pdp.def("parse_license", &PortageDepParser::parse_license,
            "parse_license(string, EAPI) -> CompositeDepSpec\n"
            "Parse a license heirarchy."
           );
    pdp.staticmethod("parse_license");
}
