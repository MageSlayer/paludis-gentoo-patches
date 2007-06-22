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
#include <libwrapiter/libwrapiter_forward_iterator.hh>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

struct RepositoryWrapper :
    Repository,
    bp::wrapper<Repository>
{
    tr1::shared_ptr<const RepositoryInfo>
    info(bool verbose) const
    {
        if (bp::override info = this->get_override("info"))
            return info(verbose);
        return Repository::info(verbose);
    }

    tr1::shared_ptr<const RepositoryInfo>
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
        tr1::shared_ptr<FSEntry> p(self.license_exists(license));
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
    /**
     * Exceptions
     */
    ExceptionRegister::get_instance()->add_exception<PackageActionError>
        ("PackageActionError", "BaseException",
         "Parent class for install, uninstall errors.");
    ExceptionRegister::get_instance()->add_exception<PackageInstallActionError>
        ("PackageInstallActionError", "PackageActionError",
         "Thrown if an install fails.");
    ExceptionRegister::get_instance()->add_exception<PackageFetchActionError>
        ("PackageFetchActionError", "PackageActionError",
         "Thrown if a fetch fails.");
    ExceptionRegister::get_instance()->add_exception<PackageUninstallActionError>
        ("PackageUninstallActionError", "PackageActionError",
         "Thrown if an uninstall fails.");
    ExceptionRegister::get_instance()->add_exception<PackageConfigActionError>
        ("PackageConfigActionError", "PackageActionError",
         "Thrown if a configure fails.");
    ExceptionRegister::get_instance()->add_exception<EnvironmentVariableActionError>
        ("EnvironmentVariableActionError", "PackageActionError",
         "Thrown if an environment variable query fails.");

    /**
     * DestinationCollection
     */
    class_collection<DestinationsCollection>
        (
         "DestinationsCollection",
         "Iterable of Repository.\n"
         "A set of Destinations."
        );

    /**
     * RepositoryInfoSection
     */
    bp::to_python_converter<std::pair<const std::string, std::string>,
            pair_to_tuple<const std::string, std::string> >();
    register_shared_ptrs_to_python<RepositoryInfoSection>();
    bp::class_<RepositoryInfoSection, boost::noncopyable>
        (
         "RepositoryInfoSection",
         "A section of information about a Repository.",
         bp::init<const std::string &>()
        )
        .add_property("heading", &RepositoryInfoSection::heading,
                "[ro] string\n"
                "Heading."
                )

        .add_property("kvs", bp::range(&RepositoryInfoSection::begin_kvs, &RepositoryInfoSection::end_kvs),
                "[ro] Iterable of tuples\n"
                "Key-value pairs."
                )
        ;

    /**
     * RepositoryInfo
     */
    register_shared_ptrs_to_python<RepositoryInfo>();
    bp::class_<RepositoryInfo, boost::noncopyable>
        (
         "RepositoryInfo",
         "Information about a Repository, for the end user.",
         bp::no_init
        )
        .add_property("sections", bp::range(&RepositoryInfo::begin_sections, &RepositoryInfo::end_sections),
                "[ro] Iterable of RepositoryInfoSection."
                )
        ;

    /**
     * Repository
     */
    register_shared_ptrs_to_python<Repository>();
    bp::class_<RepositoryWrapper, boost::noncopyable>
        (
         "Repository",
         "A Repository provides a representation of a physical repository to a PackageDatabase.",
         bp::no_init
        )
        .def("info", &Repository::info, &RepositoryWrapper::default_info,
                "info() -> RepositoryInfo"
                "Fetch information about the repository."
            )

        .add_property("name", bp::make_function(&Repository::name,
                    bp::return_value_policy<bp::copy_const_reference>()),
                "[ro] RepositoryName\n"
                "Our name."
                )

        .add_property("format", &Repository::format,
                "[ro] string\n"
                "Our format."
                )

        .def("has_category_named", &Repository::has_category_named,
                "has_category_named(CategoryNamePart) -> bool\n"
                "Do we have a category with the given name?"
            )

        .def("has_package_named", &Repository::has_package_named,
                "Do we have a package in the given category with the given name?"
            )

        .add_property("category_names", &Repository::category_names,
                "[ro] CategoryNamePartCollection\n"
                "Our category names."
                )

        .def("category_names_containing_package", &Repository::category_names_containing_package,
                "category_names_containing_package(PackageNamePart) -> CategoryNamePartCollection\n"
                "Fetch categories that contain a named package."
            )

        .def("package_names", &Repository::package_names,
                "package_names(CategoryNamePart) -> QualifiedPackageNameCollection\n"
                "Fetch our package names."
            )

        .def("version_specs", &Repository::version_specs,
                "version_specs(QualifiedPackageName) -> VersionSpecCollection\n"
                "Fetch our versions."
            )

        .def("has_version", &Repository::has_version,
                "has_version(QualifiedPackageName, VersionSpec) -> bool\n"
                "Do we have a version spec?"
            )

        .def("version_metadata", &Repository::version_metadata,
                "version_metadata(QualifiedPackageName, VersionSpec) -> VersionMetadata\n"
                "Fetch metadata."
            )

        .add_property("mask_interface", bp::make_function(&RepositoryWrapper::get_mask_interface,
                    bp::return_internal_reference<>()),
                "[ro] RepositoryMaskInterface"
                )

        .add_property("use_interface", bp::make_function(&RepositoryWrapper::get_use_interface,
                    bp::return_internal_reference<>()),
                "[ro] RepositoryUseInterface"
                )

        .add_property("installed_interface", bp::make_function(&RepositoryWrapper::get_installed_interface,
                    bp::return_internal_reference<>()),
                "[ro] RepositoryInstalledInterface"
                )

        .add_property("installable_interface", bp::make_function(&RepositoryWrapper::get_installable_interface,
                    bp::return_internal_reference<>()),
                "[ro] RepositoryInstallableInterface"
                )

        .add_property("uninstallable_interface", bp::make_function(&RepositoryWrapper::get_uninstallable_interface,
                    bp::return_internal_reference<>()),
                "[ro] RepositoryUninstallableInterface"
                )

        .add_property("sets_interface", bp::make_function(&RepositoryWrapper::get_sets_interface,
                    bp::return_internal_reference<>()),
                "[ro] RepositorySetsInterface"
                )

        .add_property("syncable_interface", bp::make_function(&RepositoryWrapper::get_syncable_interface,
                    bp::return_internal_reference<>()),
                "[ro] RepositorySyncableInterface"
                )

        .add_property("world_interface", bp::make_function(&RepositoryWrapper::get_world_interface,
                    bp::return_internal_reference<>()),
                "[ro] RepositoryWorldInterface"
                )

        .add_property("environment_variable_interface",
                bp::make_function(&RepositoryWrapper::get_environment_variable_interface,
                    bp::return_internal_reference<>()),
                "[ro] RepositoryEnvironmentInterface"
                )

        .add_property("mirrors_interface", bp::make_function(&RepositoryWrapper::get_mirrors_interface,
                    bp::return_internal_reference<>()),
                "[ro] RepositoryMirrorsInterface"
                )

        .add_property("virtuals_interface", bp::make_function(&RepositoryWrapper::get_virtuals_interface,
                    bp::return_internal_reference<>()),
                "[ro] RepositoryVirtualsInterface"
                )

        .add_property("provides_interface", bp::make_function(&RepositoryWrapper::get_provides_interface,
                    bp::return_internal_reference<>()),
                "[ro] RepositoryProvidesInterface"
                )

        .add_property("destination_interface", bp::make_function(&RepositoryWrapper::get_destination_interface,
                    bp::return_internal_reference<>()),
                "[ro] RepositoryDestinationInterface"
                )

        .add_property("contents_interface", bp::make_function(&RepositoryWrapper::get_contents_interface,
                    bp::return_internal_reference<>()),
                "[ro] RepositoryContentsInterface"
                )

        .add_property("config_interface", bp::make_function(&RepositoryWrapper::get_config_interface,
                    bp::return_internal_reference<>()),
                "[ro] RepositoryConfigInterface"
                )

        .add_property("licenses_interface", bp::make_function(&RepositoryWrapper::get_licenses_interface,
                    bp::return_internal_reference<>()),
                "[ro] RepositoryLicensesInterface"
                )

        .add_property("portage_interface", bp::make_function(&RepositoryWrapper::get_portage_interface,
                    bp::return_internal_reference<>()),
                "[ro] RepositoryPortageInterface"
                )
        ;

    /**
     * RepositoryPortageInterfaceProfilesDescLine
     */
    bp::class_<RepositoryPortageInterfaceProfilesDescLine>
        (
         "RepositoryPortageInterfaceProfilesDescLine",
         "A profiles.desc line in a Repository implementing RepositoryPortageInterface.",
         bp::no_init
        )
        .add_property("path", bp::make_getter(&RepositoryPortageInterfaceProfilesDescLine::path,
                    bp::return_value_policy<bp::return_by_value>())
                )

        .def_readonly("arch", &RepositoryPortageInterfaceProfilesDescLine::arch)

        .def_readonly("status", &RepositoryPortageInterfaceProfilesDescLine::status)
        ;

    /**
     * RepositoryMaskInterface
     */
    bp::register_ptr_to_python<RepositoryMaskInterface *>();
    bp::class_<RepositoryMaskInterface, boost::noncopyable>
        (
         "RepositoryMaskInterface",
         "Interface for handling masks for the Repository class.",
         bp::no_init
        )
        .def("query_repository_masks", &RepositoryMaskInterface::query_repository_masks,
                "query_repository_masks(QualifiedPackageName, VersionSpec) -> bool\n"
                "Query repository masks."
            )

        .def("query_profile_masks", &RepositoryMaskInterface::query_profile_masks,
                "query_profile_masks(QualifiedPackageName, VersionSpec) -> bool\n"
                "Query profile masks."
            )
        ;

    /**
     * RepositoryUseInterface
     */
    bp::register_ptr_to_python<RepositoryUseInterface *>();
    bp::class_<RepositoryUseInterface, boost::noncopyable>
        (
         "RepositoryUseInterface",
         "Interface for handling USE flags for the Repository class.",
         bp::no_init
        )
        .def("query_use", &RepositoryUseInterface::query_use,
                ("ufn", bp::arg("pde")),
                "query_use(UseFlagName, PackageDatabaseEntry) -> UseFlagState\n"
                "Query the state of the specified use flag."
            )

        .def("query_use_mask", &RepositoryUseInterface::query_use_mask,
                ("ufn", bp::arg("pde")),
                "query_use_mask(UseFlagName, PackageDatabaseEntry) -> bool\n"
                "Query whether the specified use flag is masked."
            )

        .def("query_use_force", &RepositoryUseInterface::query_use_force,
                ("ufn", bp::arg("pde")),
                "query_use_force(UseFlagName, PackageDatabaseEntry) -> bool\n"
                "Query whether the specified use flag is forceed."
            )

        .def("describe_use_flag", &RepositoryUseInterface::describe_use_flag,
                ("ufn", bp::arg("pde")),
                "describe_use_flag(UseFlagName, PackageDatabaseEntry) -> string\n"
                "Describe a use flag."
            )
        ;

    /**
     * RepositoryInstalledInterface
     */
    bp::register_ptr_to_python<RepositoryInstalledInterface *>();
    bp::class_<RepositoryInstalledInterface, boost::noncopyable>
        (
         "RepositoryInstalledInterface",
         "Interface for handling actions for installed repositories.",
         bp::no_init
        )
        .def("installed_time", &RepositoryInstalledInterfaceWrapper::installed_time,
                "installed_time(QualifiedPackageName, VersionSpec) -> datetime\n"
                "When was a package installed."
            )
        .def("root", bp::pure_virtual(&RepositoryInstalledInterface::root),
            "What is our filesystem root?"
            )
    ;

    /**
     * RepositoryInstallableInterface
     */
    bp::register_ptr_to_python<RepositoryInstallableInterface *>();
    bp::class_<RepositoryInstallableInterface, boost::noncopyable>
        (
         "RepositoryInstallableInterface",
         "Interface for handling installs for repositories.",
         bp::no_init
        );

    /**
     * RepositoryUninstallableInterface
     */
    bp::register_ptr_to_python<RepositoryUninstallableInterface *>();
    bp::class_<RepositoryUninstallableInterface, boost::noncopyable>
        (
         "RepositoryUninstallableInterface",
         "Interface for handling uninstalls for repositories.",
         bp::no_init
        );

    /**
     * RepositorySetsInterface
     */
    bp::register_ptr_to_python<RepositorySetsInterface *>();
    bp::class_<RepositorySetsInterface, boost::noncopyable>
        (
         "RepositorySetsInterface",
         "Interface for package sets for repositories.",
         bp::no_init
        );

    /**
     * RepositorySyncableInterface
     */
    bp::register_ptr_to_python<RepositorySyncableInterface *>();
    bp::class_<RepositorySyncableInterface, boost::noncopyable>
        (
         "RepositorySyncableInterface",
         "Interface for syncing for repositories.",
         bp::no_init
        );

    /**
     * RepositoryWorldInterface
     */
    bp::register_ptr_to_python<RepositoryWorldInterface *>();
    bp::class_<RepositoryWorldInterface, boost::noncopyable>
        (
         "RepositoryWorldInterface",
         "Interface for world handling for repositories.",
         bp::no_init
        );

    /**
     * RepositoryEnvironmentVariableInterface
     */
    bp::register_ptr_to_python<RepositoryEnvironmentVariableInterface *>();
    bp::class_<RepositoryEnvironmentVariableInterface, boost::noncopyable>
        (
         "RepositoryEnvironmentVariableInterface",
         "Interface for environment variable querying for repositories.",
         bp::no_init
        );

    /**
     * RepositoryMirrorsInterface
     */
    bp::register_ptr_to_python<RepositoryMirrorsInterface *>();
    bp::class_<RepositoryMirrorsInterface, boost::noncopyable>
        (
         "RepositoryMirrorsInterface",
         "Interface for mirror querying for repositories.",
         bp::no_init
        );

    /**
     * RepositoryVirtualsInterface
     */
    bp::register_ptr_to_python<RepositoryVirtualsInterface *>();
    bp::class_<RepositoryVirtualsInterface, boost::noncopyable>
        (
         "RepositoryVirtualsInterface",
         "Interface for repositories that define virtuals.",
         bp::no_init
        );

    /**
     * RepositoryProvidesInterface
     */
    bp::register_ptr_to_python<RepositoryProvidesInterface *>();
    bp::class_<RepositoryProvidesInterface, boost::noncopyable>
        (
         "RepositoryProvidesInterface",
         "Interface for repositories that provide packages.",
         bp::no_init
        );

    /**
     * RepositoryDestinationInterface
     */
    bp::register_ptr_to_python<RepositoryDestinationInterface *>();
    bp::class_<RepositoryDestinationInterface, boost::noncopyable>
        (
         "RepositoryDestinationInterface",
         "Interface for repositories that can be used as an install destination.",
         bp::no_init
        );

    /**
     * RepositoryContentsInterface
     */
    bp::register_ptr_to_python<RepositoryContentsInterface *>();
    bp::class_<RepositoryContentsInterface, boost::noncopyable>
        (
         "RepositoryContentsInterface",
         "Interface for handling actions for repositories supporting contents queries.",
         bp::no_init
        )
        .def("contents", &RepositoryContentsInterface::contents,
                "contents(QualifiedPackageName, VersionSpec) -> Contents\n"
                "Fetch contents."
            )
        ;

    /**
     * RepositoryConfigInterface
     */
    bp::register_ptr_to_python<RepositoryConfigInterface *>();
    bp::class_<RepositoryConfigInterface, boost::noncopyable>
        (
         "RepositoryConfigInterface",
         "Interface for handling actions for repositories supporting package configuration.",
         bp::no_init
        );

    /**
     * RepositoryLicensesInterface
     */
    bp::register_ptr_to_python<RepositoryLicensesInterface *>();
    bp::class_<RepositoryLicensesInterface, boost::noncopyable>
        (
         "RepositoryLicensesInterface",
         "Interface for handling actions relating to licenses.",
         bp::no_init
        )
        .def("license_exists", &RepositoryLicensesInterfaceWrapper::license_exists,
                "license_exists(string) -> path_string or None\n"
                "Check if a license exists."
            )
        ;

    /**
     * RepositoryPortageInterface
     */
    bp::register_ptr_to_python<RepositoryPortageInterface *>();
    bp::class_<RepositoryPortageInterface, boost::noncopyable>
        (
         "RepositoryPortageInterface",
         "Interface for handling PortageRepository specific functionality.",
         bp::no_init
        )
        .add_property("profiles", bp::range(&RepositoryPortageInterface::begin_profiles,
                    &RepositoryPortageInterface::end_profiles),
                "[ro] Iterable of Profiles"
                )

        .def("profile_variable", &RepositoryPortageInterface::profile_variable)

        .def("find_profile", &RepositoryPortageInterfaceWrapper::my_find_profile,
                "find_profile(path_string) -> RepositoryPortageInterfaceProfilesDescLine"
            )

        .add_property("profile", bp::object(), &RepositoryPortageInterfaceWrapper::my_set_profile,
                "[wo] RepositoryPortageInterfaceProfilesDescLine"
                )
        ;
}
