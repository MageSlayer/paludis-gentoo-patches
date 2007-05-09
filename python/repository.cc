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

#include <datetime.h>

#include <paludis/repository.hh>
#include <paludis/repositories/gentoo/portage_repository.hh>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

struct RepositoryWrapper :
    Repository,
    bp::wrapper<Repository>
{
    std::tr1::shared_ptr<const RepositoryInfo>
    info(bool verbose) const
    {
        if (bp::override info = this->get_override("info"))
            return info(verbose);
        return Repository::info(verbose);
    }

    std::tr1::shared_ptr<const RepositoryInfo>
    default_info(bool verbose) const
    {
        return Repository::info(verbose);
    }

    static RepositoryInstallableInterface *
    get_installable_interface(const Repository & self)
    {
        return self.installable_interface;
    }

    static RepositoryInstalledInterface *
    get_installed_interface(const Repository & self)
    {
        return self.installed_interface;
    }

    static RepositoryMaskInterface *
    get_mask_interface(const Repository & self)
    {
        return self.mask_interface;
    }

    static RepositorySetsInterface *
    get_sets_interface(const Repository & self)
    {
        return self.sets_interface;
    }

    static RepositorySyncableInterface *
    get_syncable_interface(const Repository & self)
    {
        return self.syncable_interface;
    }

    static RepositoryUninstallableInterface *
    get_uninstallable_interface(const Repository & self)
    {
        return self.uninstallable_interface;
    }

    static RepositoryUseInterface *
    get_use_interface(const Repository & self)
    {
        return self.use_interface;
    }

    static RepositoryWorldInterface *
    get_world_interface(const Repository & self)
    {
        return self.world_interface;
    }

    static RepositoryMirrorsInterface *
    get_mirrors_interface(const Repository & self)
    {
        return self.mirrors_interface;
    }

    static RepositoryEnvironmentVariableInterface *
    get_environment_variable_interface(const Repository & self)
    {
        return self.environment_variable_interface;
    }

    static RepositoryProvidesInterface *
    get_provides_interface(const Repository & self)
    {
        return self.provides_interface;
    }

    static RepositoryVirtualsInterface *
    get_virtuals_interface(const Repository & self)
    {
        return self.virtuals_interface;
    }

    static RepositoryDestinationInterface *
    get_destination_interface(const Repository & self)
    {
        return self.destination_interface;
    }

    static RepositoryContentsInterface *
    get_contents_interface(const Repository & self)
    {
        return self.contents_interface;
    }

    static RepositoryConfigInterface *
    get_config_interface(const Repository & self)
    {
        return self.config_interface;
    }

    static RepositoryLicensesInterface *
    get_licenses_interface(const Repository & self)
    {
        return self.licenses_interface;
    }

    static RepositoryPortageInterface *
    get_portage_interface(const Repository & self)
    {
        return self.portage_interface;
    }
};

struct RepositoryInstalledInterfaceWrapper :
    RepositoryInstalledInterface
{
    static PyObject *
    installed_time(const RepositoryInstalledInterface & self,
            const QualifiedPackageName & qpn, const VersionSpec & vs)
    {
        PyDateTime_IMPORT;
        return PyDateTime_FromTimestamp(bp::make_tuple(self.installed_time(qpn, vs)).ptr());
    }

};

struct RepositoryLicensesInterfaceWrapper :
    RepositoryLicensesInterface
{
    static PyObject *
    license_exists(const RepositoryLicensesInterface & self, const std::string & license)
    {
        std::tr1::shared_ptr<FSEntry> p(self.license_exists(license));
        if (p)
            return to_string<FSEntry>::convert(*p);
        else
            return 0;
    }
};

struct RepositoryPortageInterfaceWrapper :
    RepositoryPortageInterface
{
    static bp::object
    my_find_profile(const RepositoryPortageInterface & self, const FSEntry & location)
    {
        ProfilesIterator p(self.find_profile(location));
        if (p == self.end_profiles())
            return bp::object();
        return bp::object(bp::ptr(&*p));
    }

    static void
    my_set_profile(RepositoryPortageInterface & self, const ProfilesDescLine & pdl)
    {
        self.set_profile(self.find_profile(pdl.path));
    }

};


void PALUDIS_VISIBLE expose_repository()
{
    static register_exception<PackageActionError>
        PackageActionError("PackageActionError");
    static register_exception<PackageInstallActionError>
        PackageInstallActionError("PackageInstallActionError");
    static register_exception<PackageFetchActionError>
        PackageFetchActionError("PackageFetchActionError");
    static register_exception<PackageUninstallActionError>
        PackageUninstallActionError("PackageUninstallActionError");
    static register_exception<PackageConfigActionError>
        PackageConfigActionError("PackageConfigActionError");
    static register_exception<EnvironmentVariableActionError>
        EnvironmentVariableActionError("EnvironmentVariableActionError");

    class_collection<DestinationsCollection>
        dc("DestinationsCollection",
                "Iterable of Repository.\n"
                "A set of Destinations."

          );

    bp::to_python_converter<std::pair<const std::string, std::string>,
            pair_to_tuple<const std::string, std::string> >();
    register_shared_ptrs_to_python<RepositoryInfoSection>();
    bp::class_<RepositoryInfoSection, boost::noncopyable>
        ris("RepositoryInfoSection",
                "A section of information about a Repository.",
                bp::init<const std::string &>()
           );
    ris.add_property("heading", &RepositoryInfoSection::heading,
            "[ro] string\n"
            "Heading."
            );
    ris.add_property("kvs", bp::range(&RepositoryInfoSection::begin_kvs, &RepositoryInfoSection::end_kvs),
            "[ro] Iterable of tuples\n"
            "Key-value pairs."
            );

    register_shared_ptrs_to_python<RepositoryInfo>();
    bp::class_<RepositoryInfo, boost::noncopyable>
        ri("RepositoryInfo",
                "Information about a Repository, for the end user.",
                bp::no_init
           );
    ri.add_property("sections", bp::range(&RepositoryInfo::begin_sections, &RepositoryInfo::end_sections),
            "[ro] Iterable of RepositoryInfoSection."
            );

    register_shared_ptrs_to_python<Repository>();
    bp::class_<RepositoryWrapper, boost::noncopyable>
        r("Repository",
                "A Repository provides a representation of a physical repository to a PackageDatabase.",
                bp::no_init
         );
    r.def("info", &Repository::info, &RepositoryWrapper::default_info,
            "info() -> RepositoryInfo"
            "Fetch information about the repository."
         );
    r.add_property("name", bp::make_function(&Repository::name,
                bp::return_value_policy<bp::copy_const_reference>()),
            "[ro] RepositoryName\n"
            "Our name."
         );
    r.add_property("format", &Repository::format,
            "[ro] string\n"
            "Our format."
         );
    r.def("has_category_named", &Repository::has_category_named,
            "has_category_named(CategoryNamePart) -> bool\n"
            "Do we have a category with the given name?"
         );
    r.def("has_package_named", &Repository::has_package_named,
            "Do we have a package in the given category with the given name?"
         );
    r.add_property("category_names", &Repository::category_names,
            "[ro] CategoryNamePartCollection\n"
            "Our category names."
         );
    r.def("category_names_containing_package", &Repository::category_names_containing_package,
            "category_names_containing_package(PackageNamePart) -> CategoryNamePartCollection\n"
            "Fetch categories that contain a named package."
         );
    r.def("package_names", &Repository::package_names,
            "package_names(CategoryNamePart) -> QualifiedPackageNameCollection\n"
            "Fetch our package names."
         );
    r.def("version_specs", &Repository::version_specs,
            "version_specs(QualifiedPackageName) -> VersionSpecCollection\n"
            "Fetch our versions."
         );
    r.def("has_version", &Repository::has_version,
            "has_version(QualifiedPackageName, VersionSpec) -> bool\n"
            "Do we have a version spec?"
         );
    r.def("version_metadata", &Repository::version_metadata,
            "version_metadata(QualifiedPackageName, VersionSpec) -> VersionMetadata\n"
            "Fetch metadata."
         );
    r.add_property("mask_interface", bp::make_function(&RepositoryWrapper::get_mask_interface,
            bp::return_internal_reference<>()),
            "[ro] RepositoryMaskInterface"
            );
    r.add_property("use_interface", bp::make_function(&RepositoryWrapper::get_use_interface,
            bp::return_internal_reference<>()),
            "[ro] RepositoryUseInterface"
            );
    r.add_property("installed_interface", bp::make_function(&RepositoryWrapper::get_installed_interface,
            bp::return_internal_reference<>()),
            "[ro] RepositoryInstalledInterface"
            );
    r.add_property("installable_interface", bp::make_function(&RepositoryWrapper::get_installable_interface,
            bp::return_internal_reference<>()),
            "[ro] RepositoryInstallableInterface"
            );
    r.add_property("uninstallable_interface", bp::make_function(&RepositoryWrapper::get_uninstallable_interface,
            bp::return_internal_reference<>()),
            "[ro] RepositoryUninstallableInterface"
            );
    r.add_property("sets_interface", bp::make_function(&RepositoryWrapper::get_sets_interface,
            bp::return_internal_reference<>()),
            "[ro] RepositorySetsInterface"
            );
    r.add_property("syncable_interface", bp::make_function(&RepositoryWrapper::get_syncable_interface,
            bp::return_internal_reference<>()),
            "[ro] RepositorySyncableInterface"
            );
    r.add_property("world_interface", bp::make_function(&RepositoryWrapper::get_world_interface,
            bp::return_internal_reference<>()),
            "[ro] RepositoryWorldInterface"
            );
    r.add_property("environment_variable_interface", bp::make_function(&RepositoryWrapper::get_environment_variable_interface,
            bp::return_internal_reference<>()),
            "[ro] RepositoryEnvironmentInterface"
            );
    r.add_property("mirrors_interface", bp::make_function(&RepositoryWrapper::get_mirrors_interface,
            bp::return_internal_reference<>()),
            "[ro] RepositoryMirrorsInterface"
            );
    r.add_property("virtuals_interface", bp::make_function(&RepositoryWrapper::get_virtuals_interface,
            bp::return_internal_reference<>()),
            "[ro] RepositoryVirtualsInterface"
            );
    r.add_property("provides_interface", bp::make_function(&RepositoryWrapper::get_provides_interface,
            bp::return_internal_reference<>()),
            "[ro] RepositoryProvidesInterface"
            );
    r.add_property("destination_interface", bp::make_function(&RepositoryWrapper::get_destination_interface,
            bp::return_internal_reference<>()),
            "[ro] RepositoryDestinationInterface"
            );
    r.add_property("contents_interface", bp::make_function(&RepositoryWrapper::get_contents_interface,
            bp::return_internal_reference<>()),
            "[ro] RepositoryContentsInterface"
            );
    r.add_property("config_interface", bp::make_function(&RepositoryWrapper::get_config_interface,
            bp::return_internal_reference<>()),
            "[ro] RepositoryConfigInterface"
            );
    r.add_property("licenses_interface", bp::make_function(&RepositoryWrapper::get_licenses_interface,
            bp::return_internal_reference<>()),
            "[ro] RepositoryLicensesInterface"
            );
    r.add_property("portage_interface", bp::make_function(&RepositoryWrapper::get_portage_interface,
            bp::return_internal_reference<>()),
            "[ro] RepositoryPortageInterface"
            );


    bp::class_<RepositoryPortageInterfaceProfilesDescLine>
        rpipd("RepositoryPortageInterfaceProfilesDescLine",
                "A profiles.desc line in a Repository implementing RepositoryPortageInterface.",
                bp::no_init
             );
    rpipd.add_property("path", bp::make_getter(&RepositoryPortageInterfaceProfilesDescLine::path,
                bp::return_value_policy<bp::return_by_value>())
            );
    rpipd.def_readonly("arch", &RepositoryPortageInterfaceProfilesDescLine::arch);
    rpipd.def_readonly("status", &RepositoryPortageInterfaceProfilesDescLine::status);

    // Interfaces
    bp::register_ptr_to_python<RepositoryMaskInterface *>();
    bp::class_<RepositoryMaskInterface, boost::noncopyable>
        rmaski("RepositoryMaskInterface",
                "Interface for handling masks for the Repository class.",
                bp::no_init
           );
    rmaski.def("query_repository_masks", &RepositoryMaskInterface::query_repository_masks,
            "query_repository_masks(QualifiedPackageName, VersionSpec) -> bool\n"
            "Query repository masks."
           );
    rmaski.def("query_profile_masks", &RepositoryMaskInterface::query_profile_masks,
            "query_profile_masks(QualifiedPackageName, VersionSpec) -> bool\n"
            "Query profile masks."
           );

    bp::register_ptr_to_python<RepositoryUseInterface *>();
    bp::class_<RepositoryUseInterface, boost::noncopyable>
        rusei("RepositoryUseInterface",
                "Interface for handling USE flags for the Repository class.",
                bp::no_init
           );
    rusei.def("query_use", &RepositoryUseInterface::query_use,
            ("ufn", bp::arg("pde")=bp::object()),
            "query_use(UseFlagName, PackageDatabaseEntry=None) -> UseFlagState\n"
            "Query the state of the specified use flag."
           );
    rusei.def("query_use_mask", &RepositoryUseInterface::query_use_mask,
            ("ufn", bp::arg("pde")=bp::object()),
            "query_use_mask(UseFlagName, PackageDatabaseEntry=None) -> bool\n"
            "Query whether the specified use flag is masked."
           );
    rusei.def("query_use_force", &RepositoryUseInterface::query_use_force,
            ("ufn", bp::arg("pde")=bp::object()),
            "query_use_force(UseFlagName, PackageDatabaseEntry=None) -> bool\n"
            "Query whether the specified use flag is forceed."
           );
    rusei.def("describe_use_flag", &RepositoryUseInterface::describe_use_flag,
            ("ufn", bp::arg("pde")=bp::object()),
            "describe_use_flag(UseFlagName, PackageDatabaseEntry=None) -> string\n"
            "Describe a use flag."
           );

    bp::register_ptr_to_python<RepositoryInstalledInterface *>();
    bp::class_<RepositoryInstalledInterface, boost::noncopyable>
        rinstalledi("RepositoryInstalledInterface",
                "Interface for handling actions for installed repositories.",
                bp::no_init
             );
    rinstalledi.def("installed_time", &RepositoryInstalledInterfaceWrapper::installed_time,
            "installed_time(QualifiedPackageName, VersionSpec) -> datetime\n"
            "When was a package installed."
            );
    rinstalledi.def("root", bp::pure_virtual(&RepositoryInstalledInterface::root),
            "What is our filesystem root?"
            );

    bp::register_ptr_to_python<RepositoryInstallableInterface *>();
    bp::class_<RepositoryInstallableInterface, boost::noncopyable>
        rinstallablei("RepositoryInstallableInterface",
                "Interface for handling installs for repositories.",
                bp::no_init
               );

    bp::register_ptr_to_python<RepositoryUninstallableInterface *>();
    bp::class_<RepositoryUninstallableInterface, boost::noncopyable>
        runinstallablei("RepositoryUninstallableInterface",
                "Interface for handling uninstalls for repositories.",
                bp::no_init
         );

    bp::register_ptr_to_python<RepositorySetsInterface *>();
    bp::class_<RepositorySetsInterface, boost::noncopyable>
        rsetsi("RepositorySetsInterface",
                "Interface for package sets for repositories.",
                bp::no_init
         );

    bp::register_ptr_to_python<RepositorySyncableInterface *>();
    bp::class_<RepositorySyncableInterface, boost::noncopyable>
        rsynci("RepositorySyncableInterface",
                "Interface for syncing for repositories.",
                bp::no_init
         );

    bp::register_ptr_to_python<RepositoryWorldInterface *>();
    bp::class_<RepositoryWorldInterface, boost::noncopyable>
        rworldi("RepositoryWorldInterface",
                "Interface for world handling for repositories.",
                bp::no_init
         );

    bp::register_ptr_to_python<RepositoryEnvironmentVariableInterface *>();
    bp::class_<RepositoryEnvironmentVariableInterface, boost::noncopyable>
        renvvari("RepositoryEnvironmentVariableInterface",
                "Interface for environment variable querying for repositories.",
                bp::no_init
         );

    bp::register_ptr_to_python<RepositoryMirrorsInterface *>();
    bp::class_<RepositoryMirrorsInterface, boost::noncopyable>
        rmirrorsi("RepositoryMirrorsInterface",
                "Interface for mirror querying for repositories.",
                bp::no_init
         );

    bp::register_ptr_to_python<RepositoryVirtualsInterface *>();
    bp::class_<RepositoryVirtualsInterface, boost::noncopyable>
        rvirtualsi("RepositoryVirtualsInterface",
                "Interface for repositories that define virtuals.",
                bp::no_init
         );

    bp::register_ptr_to_python<RepositoryProvidesInterface *>();
    bp::class_<RepositoryProvidesInterface, boost::noncopyable>
        rprovidesi("RepositoryProvidesInterface",
                "Interface for repositories that provide packages.",
                bp::no_init
         );

    bp::register_ptr_to_python<RepositoryDestinationInterface *>();
    bp::class_<RepositoryDestinationInterface, boost::noncopyable>
        rdesti("RepositoryDestinationInterface",
                "Interface for repositories that can be used as an install destination.",
                bp::no_init
         );

    bp::register_ptr_to_python<RepositoryContentsInterface *>();
    bp::class_<RepositoryContentsInterface, boost::noncopyable>
        rcontentsi("RepositoryContentsInterface",
                "Interface for handling actions for repositories supporting contents queries.",
                bp::no_init
         );
    rcontentsi.def("contents", &RepositoryContentsInterface::contents,
            "contents(QualifiedPackageName, VersionSpec) -> Contents\n"
            "Fetch contents."
            );

    bp::register_ptr_to_python<RepositoryConfigInterface *>();
    bp::class_<RepositoryConfigInterface, boost::noncopyable>
        rconfigi("RepositoryConfigInterface",
                "Interface for handling actions for repositories supporting package configuration.",
                bp::no_init
         );

    bp::register_ptr_to_python<RepositoryLicensesInterface *>();
    bp::class_<RepositoryLicensesInterface, boost::noncopyable>
        rlici("RepositoryLicensesInterface",
                "Interface for handling actions relating to licenses.",
                bp::no_init
         );
    rlici.def("license_exists", &RepositoryLicensesInterfaceWrapper::license_exists,
            "license_exists(string) -> path_string or None\n"
            "Check if a license exists."
            );

    bp::register_ptr_to_python<RepositoryPortageInterface *>();
    bp::class_<RepositoryPortageInterface, boost::noncopyable>
        rporti("RepositoryPortageInterface",
                "Interface for handling PortageRepository specific functionality.",
                bp::no_init
         );
    rporti.add_property("profiles", bp::range(&RepositoryPortageInterface::begin_profiles,
                &RepositoryPortageInterface::end_profiles),
            "[ro] Iterable of Profiles"
            );
    rporti.def("profile_variable", &RepositoryPortageInterface::profile_variable);
    rporti.def("find_profile", &RepositoryPortageInterfaceWrapper::my_find_profile,
            "find_profile(path_string) -> RepositoryPortageInterfaceProfilesDescLine"
            );
    rporti.add_property("profile", bp::object(), &RepositoryPortageInterfaceWrapper::my_set_profile,
            "[wo] RepositoryPortageInterfaceProfilesDescLine"
            );
}
