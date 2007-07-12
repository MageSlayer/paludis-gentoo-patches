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

#include <paludis/repository.hh>
#include <paludis/repository_info.hh>
#include <paludis/repositories/e/e_repository.hh>
#include <paludis/repositories/fake/fake_repository.hh>
#include <paludis/repositories/fake/fake_package_id.hh>
#include <paludis/package_id.hh>
#include <paludis/environment.hh>
#include <paludis/util/set.hh>
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

    static RepositoryLicensesInterface *
    get_licenses_interface(const Repository & self)
    {
        return self.licenses_interface;
    }

    static RepositoryEInterface *
    get_e_interface(const Repository & self)
    {
        return self.e_interface;
    }
};

struct FakeRepositoryWrapper :
    FakeRepository,
    bp::wrapper<FakeRepository>
{
    FakeRepositoryWrapper(const Environment * const env, const RepositoryName & name) :
        FakeRepository(env, name)
    {
    }

    static tr1::shared_ptr<PackageID>
    add_version(FakeRepository & self, const QualifiedPackageName & qpn, const VersionSpec & vs)
    {
        return self.add_version(qpn, vs);
    }
};

struct RepositoryLicensesInterfaceWrapper
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

struct RepositoryEInterfaceWrapper
{
    static bp::object
    my_find_profile(const RepositoryEInterface & self, const FSEntry & location)
    {
        RepositoryEInterface::ProfilesIterator p(self.find_profile(location));
        if (p == self.end_profiles())
            return bp::object();
        return bp::object(bp::ptr(&*p));
    }

    static void
    my_set_profile(RepositoryEInterface & self, const RepositoryEInterface::ProfilesDescLine & pdl)
    {
        self.set_profile(self.find_profile(pdl.path));
    }
};

// FIXME
//template <typename I_>
//struct repository_interface_to_python
//{
//    static PyObject *
//    convert(const I_ & i)
//    {
//        return 0;
//        return bp::incref(bp::object(bp::ptr(&i)).ptr());
//    }
//};
//
//template <typename I_>
//void register_repository_interface_to_python()
//{
//    bp::to_python_converter<I_, repository_interface_to_python<I_> >();
//}

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
     * DestinationIterable
     */
    class_iterable<DestinationsSet>
        (
         "DestinationsIterable",
         "Iterable of Repository."
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
    register_shared_ptrs_to_python<Repository>(rsp_const);
    bp::class_<RepositoryWrapper, tr1::shared_ptr<Repository>, boost::noncopyable>
        (
         "Repository",
         "A Repository provides a representation of a physical repository to a PackageDatabase.",
         bp::no_init
        )
        .def("info", &Repository::info, &RepositoryWrapper::default_info,
                "info() -> RepositoryInfo\n"
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
                "has_package_named(QualifiedPackageName) -> bool\n"
                "Do we have a package in the given category with the given name?"
            )

        .add_property("category_names", &Repository::category_names,
                "[ro] CategoryNamePartIterable\n"
                "Our category names."
                )

        .def("category_names_containing_package", &Repository::category_names_containing_package,
                "category_names_containing_package(PackageNamePart) -> CategoryNamePartIterable\n"
                "Fetch categories that contain a named package."
            )

        .def("package_names", &Repository::package_names,
                "package_names(CategoryNamePart) -> QualifiedPackageNameIterable\n"
                "Fetch our package names."
            )

        .def("package_ids", &Repository::package_ids, bp::with_custodian_and_ward_postcall<0, 1>(),
                "package_ids(QualifiedPackageName) -> PackageIDIterable\n"
                "Fetch our versions."
            )

        .add_property("mask_interface", bp::make_function(&RepositoryWrapper::get_mask_interface,
                    bp::return_internal_reference<>()),
                "[ro] RepositoryMaskInterface"
                )

        .add_property("use_interface", bp::make_function(&RepositoryWrapper::get_use_interface,
                    bp::return_internal_reference<>()),
                "[ro] RepositoryUseInterface"
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

        .add_property("licenses_interface", bp::make_function(&RepositoryWrapper::get_licenses_interface,
                    bp::return_internal_reference<>()),
                "[ro] RepositoryLicensesInterface"
                )

        .add_property("e_interface", bp::make_function(&RepositoryWrapper::get_e_interface,
                    bp::return_internal_reference<>()),
                "[ro] RepositoryEInterface"
                )
        ;

    /**
     * RepositoryEInterfaceProfilesDescLine
     */
    bp::class_<RepositoryEInterfaceProfilesDescLine>
        (
         "RepositoryEInterfaceProfilesDescLine",
         "A profiles.desc line in a Repository implementing RepositoryEInterface.",
         bp::no_init
        )
        .add_property("path", bp::make_getter(&RepositoryEInterfaceProfilesDescLine::path,
                    bp::return_value_policy<bp::return_by_value>())
                )

        .def_readonly("arch", &RepositoryEInterfaceProfilesDescLine::arch)

        .def_readonly("status", &RepositoryEInterfaceProfilesDescLine::status)
        ;

    /**
     * RepositoryMaskInterface
     */
    bp::class_<RepositoryMaskInterface, boost::noncopyable>
        (
         "RepositoryMaskInterface",
         "Interface for handling masks for the Repository class.",
         bp::no_init
        )
        .def("query_repository_masks", &RepositoryMaskInterface::query_repository_masks,
                "query_repository_masks(PackageID) -> bool\n"
                "Query repository masks."
            )

        .def("query_profile_masks", &RepositoryMaskInterface::query_profile_masks,
                "query_profile_masks(PackageID) -> bool\n"
                "Query profile masks."
            )
        ;

    /**
     * RepositoryUseInterface
     */
    bp::class_<RepositoryUseInterface, boost::noncopyable>
        (
         "RepositoryUseInterface",
         "Interface for handling USE flags for the Repository class.",
         bp::no_init
        )
        .def("query_use", &RepositoryUseInterface::query_use,
                ("ufn", bp::arg("pde")),
                "query_use(UseFlagName, PackageID) -> UseFlagState\n"
                "Query the state of the specified use flag."
            )

        .def("query_use_mask", &RepositoryUseInterface::query_use_mask,
                ("ufn", bp::arg("pde")),
                "query_use_mask(UseFlagName, PackageID) -> bool\n"
                "Query whether the specified use flag is masked."
            )

        .def("query_use_force", &RepositoryUseInterface::query_use_force,
                ("ufn", bp::arg("pde")),
                "query_use_force(UseFlagName, PackageID) -> bool\n"
                "Query whether the specified use flag is forceed."
            )

        .def("describe_use_flag", &RepositoryUseInterface::describe_use_flag,
                ("ufn", bp::arg("pde")),
                "describe_use_flag(UseFlagName, PackageID) -> string\n"
                "Describe a use flag."
            )
        ;

    /**
     * RepositoryInstalledInterface
     */
    bp::class_<RepositoryInstalledInterface, boost::noncopyable>
        (
         "RepositoryInstalledInterface",
         "Interface for handling actions for installed repositories.",
         bp::no_init
        )
        .def("root", bp::pure_virtual(&RepositoryInstalledInterface::root),
            "What is our filesystem root?"
            )
    ;

    /**
     * RepositorySetsInterface
     */
    bp::class_<RepositorySetsInterface, boost::noncopyable>
        (
         "RepositorySetsInterface",
         "Interface for package sets for repositories.",
         bp::no_init
        );

    /**
     * RepositorySyncableInterface
     */
    bp::class_<RepositorySyncableInterface, boost::noncopyable>
        (
         "RepositorySyncableInterface",
         "Interface for syncing for repositories.",
         bp::no_init
        );

    /**
     * RepositoryWorldInterface
     */
    bp::class_<RepositoryWorldInterface, boost::noncopyable>
        (
         "RepositoryWorldInterface",
         "Interface for world handling for repositories.",
         bp::no_init
        );

    /**
     * RepositoryEnvironmentVariableInterface
     */
    bp::class_<RepositoryEnvironmentVariableInterface, boost::noncopyable>
        (
         "RepositoryEnvironmentVariableInterface",
         "Interface for environment variable querying for repositories.",
         bp::no_init
        );

    /**
     * RepositoryMirrorsInterface
     */
    bp::class_<RepositoryMirrorsInterface, boost::noncopyable>
        (
         "RepositoryMirrorsInterface",
         "Interface for mirror querying for repositories.",
         bp::no_init
        );

    /**
     * RepositoryVirtualsInterface
     */
    bp::class_<RepositoryVirtualsInterface, boost::noncopyable>
        (
         "RepositoryVirtualsInterface",
         "Interface for repositories that define virtuals.",
         bp::no_init
        );

    /**
     * RepositoryProvidesInterface
     */
    bp::class_<RepositoryProvidesInterface, boost::noncopyable>
        (
         "RepositoryProvidesInterface",
         "Interface for repositories that provide packages.",
         bp::no_init
        );

    /**
     * RepositoryDestinationInterface
     */
    bp::class_<RepositoryDestinationInterface, boost::noncopyable>
        (
         "RepositoryDestinationInterface",
         "Interface for repositories that can be used as an install destination.",
         bp::no_init
        );

    /**
     * RepositoryLicensesInterface
     */
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
     * RepositoryEInterface
     */
    bp::class_<RepositoryEInterface, boost::noncopyable>
        (
         "RepositoryEInterface",
         "Interface for handling ERepository specific functionality.",
         bp::no_init
        )
        .add_property("profiles", bp::range(&RepositoryEInterface::begin_profiles,
                    &RepositoryEInterface::end_profiles),
                "[ro] Iterable of Profiles"
                )

        .def("profile_variable", &RepositoryEInterface::profile_variable)

        .def("find_profile", &RepositoryEInterfaceWrapper::my_find_profile,
                "find_profile(path_string) -> RepositoryEInterfaceProfilesDescLine"
            )

        .add_property("profile", bp::object(), &RepositoryEInterfaceWrapper::my_set_profile,
                "[wo] RepositoryEInterfaceProfilesDescLine"
                )
        ;

    /**
     * FakeRepository
     */
    tr1::shared_ptr<FakePackageID>
        (FakeRepository:: *add_version_ptr)(const QualifiedPackageName &, const VersionSpec &) =
        &FakeRepository::add_version;

    bp::class_<FakeRepositoryWrapper, tr1::shared_ptr<FakeRepository>, bp::bases<Repository>, boost::noncopyable>
        (
         "FakeRepository",
         "Fake repository for use in test cases.",
         bp::init<const Environment * const, const RepositoryName &>("__init__(Environment, RepositoryName)")
        )
        .def("add_category", &FakeRepository::add_category)

        .def("add_package", &FakeRepository::add_package)

        .def("add_version", &FakeRepositoryWrapper::add_version)
        ;
}
