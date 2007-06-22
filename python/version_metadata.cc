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

#include <paludis/version_metadata.hh>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

struct VersionMetadataWrapper
{
    static VersionMetadataEbuildInterface *
    get_ebuild_interface(const VersionMetadata & self)
    {
        return self.ebuild_interface;
    }

    static VersionMetadataEbinInterface *
    get_ebin_interface(const VersionMetadata & self)
    {
        return self.ebin_interface;
    }

    static VersionMetadataCRANInterface *
    get_cran_interface(const VersionMetadata & self)
    {
        return self.cran_interface;
    }

    static VersionMetadataDepsInterface *
    get_deps_interface(const VersionMetadata & self)
    {
        return self.deps_interface;
    }

    static VersionMetadataOriginsInterface *
    get_origins_interface(const VersionMetadata & self)
    {
        return self.origins_interface;
    }

    static VersionMetadataVirtualInterface *
    get_virtual_interface(const VersionMetadata & self)
    {
        return self.virtual_interface;
    }

    static VersionMetadataLicenseInterface *
    get_license_interface(const VersionMetadata & self)
    {
        return self.license_interface;
    }
};

void PALUDIS_VISIBLE expose_version_metadata()
{
    /**
     * VersionMetadata
     */
    bp::register_ptr_to_python<tr1::shared_ptr<const VersionMetadata> >();
    bp::class_<VersionMetadata, boost::noncopyable>
        (
         "VersionMetadata",
         "Version metadata.",
         bp::no_init
        )
        .def_readonly("slot", &VersionMetadata::slot,
                "[ro] SlotName"
                )

        .def_readonly("homepage", &VersionMetadata::homepage,
                "[ro] DepSpec"
                )

        .def_readonly("description", &VersionMetadata::description,
                "[ro] string"
                )

        .def_readonly("eapi", &VersionMetadata::eapi,
                "[ro] EAPI"
                )

        .add_property("ebuild_interface", bp::make_function(&VersionMetadataWrapper::get_ebuild_interface,
                    bp::return_internal_reference<>()),
                "[ro] EbuildInterface"
                )

        .add_property("ebin_interface", bp::make_function(&VersionMetadataWrapper::get_ebin_interface,
                    bp::return_internal_reference<>()),
                "[ro] EbinInterface"
                )

        .add_property("cran_interface", bp::make_function(&VersionMetadataWrapper::get_cran_interface,
                    bp::return_internal_reference<>()),
                "[ro] CRANInterface"
                )

        .add_property("deps_interface", bp::make_function(&VersionMetadataWrapper::get_deps_interface,
                    bp::return_internal_reference<>()),
                "[ro] DepsInterface"
                )

        .add_property("origins_interface", bp::make_function(&VersionMetadataWrapper::get_origins_interface,
                    bp::return_internal_reference<>()),
                "[ro] OriginsInterface"
                )

        .add_property("virtual_interface", bp::make_function(&VersionMetadataWrapper::get_virtual_interface,
                    bp::return_internal_reference<>()),
                "[ro] VirtualInterface"
                )

        .add_property("license_interface", bp::make_function(&VersionMetadataWrapper::get_license_interface,
                    bp::return_internal_reference<>()),
                "[ro] LicenseInterface"
                )
        ;

    /**
     * VersionMetadataEbuildInterface
     */
    bp::register_ptr_to_python<VersionMetadataEbuildInterface *>();
    bp::class_<VersionMetadataEbuildInterface, boost::noncopyable>
        (
         "VersionMetadataEbuildInterface",
         "Version metadata for ebuilds.",
         bp::no_init
        )
        .def_readonly("provide", &VersionMetadataEbuildInterface::provide,
                "[ro] DepSpec"
                )

        .def_readonly("src_uri", &VersionMetadataEbuildInterface::src_uri,
                "[ro] DepSpec"
                )

        .def_readonly("restrictions", &VersionMetadataEbuildInterface::restrictions,
                "[ro] DepSpec"
                )

        .def_readonly("keywords", &VersionMetadataEbuildInterface::keywords,
                "[ro] KeywordNameCollection"
                )

        .def_readonly("eclass_keywords", &VersionMetadataEbuildInterface::eclass_keywords,
                "[ro] string"
                )

        .def_readonly("iuse", &VersionMetadataEbuildInterface::iuse,
                "[ro] IUseFlagCollection"
                )

        .def_readonly("inherited", &VersionMetadataEbuildInterface::inherited,
                "[ro] InheritedCollection"
                )
        ;

    /**
     * VersionMetadataEbinInterface
     */
    bp::register_ptr_to_python<VersionMetadataEbinInterface *>();
    bp::class_<VersionMetadataEbinInterface, boost::noncopyable>
        (
         "VersionMetadataEbinInterface",
         "Version metadata for Ebins.",
         bp::no_init
        )
        .def_readonly("bin_uri", &VersionMetadataEbinInterface::bin_uri,
                "[ro] DepSpec"
                )
        ;

    /**
     * VersionMetadataCRANInterface
     */
    bp::register_ptr_to_python<VersionMetadataCRANInterface *>();
    bp::class_<VersionMetadataCRANInterface, boost::noncopyable>
        (
         "VersionMetadataCRANInterface",
         "Version metadata for CRAN packages.",
         bp::no_init
        )
        .def_readonly("keywords", &VersionMetadataCRANInterface::keywords,
                "[ro] string"
                )

        .def_readonly("package", &VersionMetadataCRANInterface::package,
                "[ro] string"
                )

        .def_readonly("version", &VersionMetadataCRANInterface::version,
                "[ro] string"
                )
        ;

    /**
     * VersionMetadataDepsInterface
     */
    bp::register_ptr_to_python<VersionMetadataDepsInterface *>();
    bp::class_<VersionMetadataDepsInterface, boost::noncopyable>
        (
         "VersionMetadataDepsInterface",
         "Dependency data for VersionMetadata.",
         bp::no_init
        )
        .add_property("build_depend", &VersionMetadataDepsInterface::build_depend,
                "[ro] DepSpec"
                )

        .add_property("run_depend", &VersionMetadataDepsInterface::run_depend,
                "[ro] DepSpec"
                )

        .add_property("post_depend", &VersionMetadataDepsInterface::post_depend,
                "[ro] DepSpec"
                )

        .add_property("suggested_depend", &VersionMetadataDepsInterface::suggested_depend,
                "[ro] DepSpec"
                )
        ;

    /**
     * VersionMetadataOriginsInterface
     */
    bp::register_ptr_to_python<VersionMetadataOriginsInterface *>();
    bp::class_<VersionMetadataOriginsInterface, boost::noncopyable>
        (
         "VersionMetadataOriginsInterface",
         "Origins data for VersionMetadata.",
         bp::no_init
        )
        .add_property("source", bp::make_getter(&VersionMetadataOriginsInterface::source,
                    bp::return_value_policy<bp::return_by_value>()),
                "[ro] PackageDatabaseEntry"
                )

        .add_property("binary", bp::make_getter(&VersionMetadataOriginsInterface::binary,
                    bp::return_value_policy<bp::return_by_value>()),
                "[ro] PackageDatabaseEntry"
                )
        ;

    /**
     * VersionMetadataCRANInterface
     */
    bp::register_ptr_to_python<VersionMetadataVirtualInterface *>();
    bp::class_<VersionMetadataVirtualInterface, boost::noncopyable>
        (
         "VersionMetadataVirtualInterface",
         "Version metadata for virtual packages.",
         bp::no_init
        )
        .def_readonly("virtual_for", &VersionMetadataVirtualInterface::virtual_for,
                "[ro] PackageDatabaseEntry"
                )
        ;

    /**
     * VersionMetadataCRANInterface
     */
    bp::register_ptr_to_python<VersionMetadataLicenseInterface *>();
    bp::class_<VersionMetadataLicenseInterface, boost::noncopyable>
        (
         "VersionMetadataLicenseInterface",
         "License data for VersionMetadata.",
         bp::no_init
        )
        .def_readonly("license", &VersionMetadataLicenseInterface::license,
                "[ro] DepSpec"
                )
        ;
}

