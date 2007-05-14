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
    bp::register_ptr_to_python<std::tr1::shared_ptr<const VersionMetadata> >();
    bp::class_<VersionMetadata, boost::noncopyable>
        vm("VersionMetadata",
                "Version metadata.",
                bp::no_init
          );
    vm.def_readonly("slot", &VersionMetadata::slot,
            "[ro] SlotName"
            );
    vm.def_readonly("homepage", &VersionMetadata::homepage,
            "[ro] DepSpec"
            );
    vm.def_readonly("description", &VersionMetadata::description,
            "[ro] string"
            );
#ifdef CIARANM_REMOVED_THIS
    vm.def_readonly("eapi", &VersionMetadata::eapi,
            "[ro] string"
            );
#endif
    vm.add_property("ebuild_interface", bp::make_function(&VersionMetadataWrapper::get_ebuild_interface,
            bp::return_internal_reference<>()),
            "[ro] EbuildInterface"
            );
    vm.add_property("ebin_interface", bp::make_function(&VersionMetadataWrapper::get_ebin_interface,
            bp::return_internal_reference<>()),
            "[ro] EbinInterface"
            );
    vm.add_property("cran_interface", bp::make_function(&VersionMetadataWrapper::get_cran_interface,
            bp::return_internal_reference<>()),
            "[ro] CRANInterface"
            );
    vm.add_property("deps_interface", bp::make_function(&VersionMetadataWrapper::get_deps_interface,
            bp::return_internal_reference<>()),
            "[ro] DepsInterface"
            );
    vm.add_property("origins_interface", bp::make_function(&VersionMetadataWrapper::get_origins_interface,
            bp::return_internal_reference<>()),
            "[ro] OriginsInterface"
            );
    vm.add_property("virtual_interface", bp::make_function(&VersionMetadataWrapper::get_virtual_interface,
            bp::return_internal_reference<>()),
            "[ro] VirtualInterface"
            );
    vm.add_property("license_interface", bp::make_function(&VersionMetadataWrapper::get_license_interface,
            bp::return_internal_reference<>()),
            "[ro] LicenseInterface"
            );

    bp::register_ptr_to_python<VersionMetadataEbuildInterface *>();
    bp::class_<VersionMetadataEbuildInterface, boost::noncopyable>
        ebuild_i("VersionMetadataEbuildInterface",
                "Version metadata for ebuilds.",
                bp::no_init
                );

    ebuild_i.def_readonly("provide", &VersionMetadataEbuildInterface::provide,
            "[ro] DepSpec"
            );
    ebuild_i.def_readonly("src_uri", &VersionMetadataEbuildInterface::src_uri,
            "[ro] DepSpec"
            );
    ebuild_i.def_readonly("restrictions", &VersionMetadataEbuildInterface::restrictions,
            "[ro] DepSpec"
            );
    ebuild_i.def_readonly("keywords", &VersionMetadataEbuildInterface::keywords,
            "[ro] KeywordNameCollection"
            );
    ebuild_i.def_readonly("eclass_keywords", &VersionMetadataEbuildInterface::eclass_keywords,
            "[ro] string"
            );
    ebuild_i.def_readonly("iuse", &VersionMetadataEbuildInterface::iuse,
            "[ro] IUseFlagCollection"
            );
    ebuild_i.def_readonly("inherited", &VersionMetadataEbuildInterface::inherited,
            "[ro] InheritedCollection"
            );

    bp::register_ptr_to_python<VersionMetadataEbinInterface *>();
    bp::class_<VersionMetadataEbinInterface, boost::noncopyable>
        ebin_i("VersionMetadataEbinInterface",
                "Version metadata for Ebins.",
                bp::no_init
              );
    ebin_i.def_readonly("bin_uri", &VersionMetadataEbinInterface::bin_uri,
            "[ro] DepSpec"
            );

    bp::register_ptr_to_python<VersionMetadataCRANInterface *>();
    bp::class_<VersionMetadataCRANInterface, boost::noncopyable>
        cran_i("VersionMetadataCRANInterface",
                "Version metadata for CRAN packages.",
                bp::no_init
                );
    cran_i.def_readonly("keywords", &VersionMetadataCRANInterface::keywords,
            "[ro] string"
            );
    cran_i.def_readonly("package", &VersionMetadataCRANInterface::package,
            "[ro] string"
            );
    cran_i.def_readonly("version", &VersionMetadataCRANInterface::version,
            "[ro] string"
            );

    bp::register_ptr_to_python<VersionMetadataDepsInterface *>();
    bp::class_<VersionMetadataDepsInterface, boost::noncopyable>
        deps_i("VersionMetadataDepsInterface",
                "Dependency data for VersionMetadata.",
                bp::no_init
                );
    deps_i.add_property("build_depend", &VersionMetadataDepsInterface::build_depend,
            "[ro] DepSpec"
            );
    deps_i.add_property("run_depend", &VersionMetadataDepsInterface::run_depend,
            "[ro] DepSpec"
            );
    deps_i.add_property("post_depend", &VersionMetadataDepsInterface::post_depend,
            "[ro] DepSpec"
            );
    deps_i.add_property("suggested_depend", &VersionMetadataDepsInterface::suggested_depend,
            "[ro] DepSpec"
            );

    bp::register_ptr_to_python<VersionMetadataOriginsInterface *>();
    bp::class_<VersionMetadataOriginsInterface, boost::noncopyable>
        origins_i("VersionMetadataOriginsInterface",
                "Origins data for VersionMetadata.",
                bp::no_init
                );
    origins_i.add_property("source", bp::make_getter(&VersionMetadataOriginsInterface::source,
                bp::return_value_policy<bp::return_by_value>()),
            "[ro] PackageDatabaseEntry"
            );
    origins_i.add_property("binary", bp::make_getter(&VersionMetadataOriginsInterface::binary,
                bp::return_value_policy<bp::return_by_value>()),
            "[ro] PackageDatabaseEntry"
            );

    bp::register_ptr_to_python<VersionMetadataVirtualInterface *>();
    bp::class_<VersionMetadataVirtualInterface, boost::noncopyable>
        virtual_i("VersionMetadataVirtualInterface",
                "Version metadata for virtual packages.",
                bp::no_init
                );
    virtual_i.def_readonly("virtual_for", &VersionMetadataVirtualInterface::virtual_for,
            "[ro] PackageDatabaseEntry"
            );

    bp::register_ptr_to_python<VersionMetadataLicenseInterface *>();
    bp::class_<VersionMetadataLicenseInterface, boost::noncopyable>
        license_i("VersionMetadataLicenseInterface",
                "License data for VersionMetadata.",
                bp::no_init
                );
    license_i.def_readonly("license", &VersionMetadataLicenseInterface::license,
            "[ro] DepSpec"
            );
}

