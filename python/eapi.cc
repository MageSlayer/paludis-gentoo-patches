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

#include <paludis/eapi.hh>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

void expose_eapi()
{
    register_shared_ptrs_to_python<SupportedEAPI>();
    bp::class_<SupportedEAPI, boost::noncopyable>
        se("SupportedEAPI",
                "Information about a supported EAPI.",
                bp::no_init
          );
    se.def_readonly("package_dep_spec_parse_mode", &SupportedEAPI::package_dep_spec_parse_mode,
            "[ro] PackageDepSpecParseMode"
            );
    se.def_readonly("strict_package_dep_spec_parse_mode", &SupportedEAPI::strict_package_dep_spec_parse_mode,
            "[ro] PackageDepSpecParseMode"
            );
    se.def_readonly("iuse_flag_parse_mode", &SupportedEAPI::iuse_flag_parse_mode,
            "[ro] IUseFlagParseMode"
            );
    se.def_readonly("strict_iuse_flag_parse_mode", &SupportedEAPI::strict_iuse_flag_parse_mode,
            "[ro] IUseFlagParseMode"
            );
    se.def_readonly("breaks_portage", &SupportedEAPI::breaks_portage,
            "[ro] bool"
            );
    se.def_readonly("has_pretend_phase", &SupportedEAPI::has_pretend_phase,
            "[ro] bool"
            );

    bp::class_<EAPI>
        e("EAPI",
                "Information about an EAPI.",
                bp::no_init
         );
    e.def_readonly("name", &EAPI::name,
            "[ro] str"
            );
    e.add_property("supported", bp::make_getter(&EAPI::supported, bp::return_value_policy<bp::return_by_value>()),
            "[ro] SupportedEAPI"
            );

    bp::class_<EAPIData, boost::noncopyable>
        ed("EAPIData",
                "Holds information on recognised EAPIs.",
                bp::no_init
          );
    ed.add_static_property("instance", bp::make_function(&EAPIData::get_instance,
                bp::return_value_policy<bp::reference_existing_object>()),
            "Singleton instance."
            );
    ed.def("eapi_from_string", &EAPIData::eapi_from_string,
            "eapi_from_string(str) -> EAPI\n"
            "Make an EAPI."
          );
    ed.def("unknown_eapi", &EAPIData::unknown_eapi,
            "unknown_eapi() -> EAPI\n"
            "Make the unknown EAPI."
          );
}
