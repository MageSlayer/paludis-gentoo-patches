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
#include <python/iterable.hh>

#include <paludis/repository.hh>
#include <paludis/repositories/e/e_repository.hh>
#include <paludis/repositories/fake/fake_repository.hh>
#include <paludis/repositories/fake/fake_package_id.hh>
#include <paludis/action.hh>
#include <paludis/package_id.hh>
#include <paludis/environment.hh>
#include <paludis/util/options.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/make_named_values.hh>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

struct RepositoryWrapper :
    Repository,
    bp::wrapper<Repository>
{
    static RepositoryEnvironmentVariableInterface *
    get_environment_variable_interface(const Repository & self)
    {
        return self.environment_variable_interface();
    }

    static RepositoryProvidesInterface *
    get_provides_interface(const Repository & self)
    {
        return self.provides_interface();
    }

    static RepositoryVirtualsInterface *
    get_virtuals_interface(const Repository & self)
    {
        return self.virtuals_interface();
    }

    static RepositoryDestinationInterface *
    get_destination_interface(const Repository & self)
    {
        return self.destination_interface();
    }

    static PyObject *
    find_metadata(const Repository & self, const std::string & key)
    {
        Repository::MetadataConstIterator i(self.find_metadata(key));
        if (i != self.end_metadata())
            return bp::incref(bp::object(*i).ptr());
        else
            return Py_None;
    }
};

struct FakeRepositoryWrapper
{
    static std::shared_ptr<PackageID>
    add_version(FakeRepository & self, const QualifiedPackageName & qpn, const VersionSpec & vs)
    {
        return self.add_version(qpn, vs);
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

namespace
{
    std::shared_ptr<FakeRepository>
    make_fake_repository(const Environment * const env, const RepositoryName & n)
    {
        return std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                        n::environment() = env,
                        n::name() = n
                        ));
    }
}

void expose_repository()
{
    /**
     * DestinationIterable
     */
    class_iterable<DestinationsSet>
        (
         "DestinationsIterable",
         "Iterable of Repository.",
         true
        );

    /**
     * Repository
     */
    register_shared_ptrs_to_python<Repository>(rsp_const);
    bp::class_<RepositoryWrapper, std::shared_ptr<Repository>, boost::noncopyable>
        (
         "Repository",
         "A Repository provides a representation of a physical repository to a PackageDatabase.",
         bp::no_init
        )

        .add_property("name", &Repository::name,
                "[ro] RepositoryName\n"
                "Our name."
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

        .def("some_ids_might_support_action", &Repository::some_ids_might_support_action,
                "some_ids_might_support_action(SupportsActionTestBase) -> bool\n"
                "Might some of our IDs support a particular action?\n\n"

                "Used to optimise PackageDatabase::query. If a repository doesn't\n"
                "support, say, InstallAction, a query can skip searching it\n"
                "entirely when looking for installable packages."
            )

        .add_property("environment_variable_interface",
                bp::make_function(&RepositoryWrapper::get_environment_variable_interface,
                    bp::return_internal_reference<>()),
                "[ro] RepositoryEnvironmentInterface"
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

        .def("format_key", &Repository::format_key,
                "The format_key, if not None, holds our repository's format"
            )

        .def("location_key", &Repository::location_key,
                "The location_key, if not None, holds a file or directory containing "
                "our repository's data."
            )

        .def("installed_root_key", &Repository::installed_root_key,
                "The installed_root_key, if not None, specifies that we contain installed "
                "packages at the specified root."
            )

        .add_property("metadata", bp::range(&Repository::begin_metadata, &Repository::end_metadata),
                "[ro] Iterable of MetadataKey\n"
                "NEED_DOC"
                )

        .def("find_metadata", &RepositoryWrapper::find_metadata,
                "find_metadata(string) -> MetadataKey\n"
                "NEED_DOC"
            )
        ;

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
     * FakeRepository
     */
    bp::implicitly_convertible<std::shared_ptr<FakeRepository>, std::shared_ptr<Repository> >();

    bp::class_<FakeRepository, std::shared_ptr<FakeRepository>, bp::bases<Repository>, boost::noncopyable>
        (
         "FakeRepository",
         "Fake repository for use in test cases.",
         bp::no_init
        )

        .def("__init__",
                bp::make_constructor(&make_fake_repository),
                "__init__(Environment, RepositoryName)"
            )

        .def("add_category", &FakeRepository::add_category)

        .def("add_package", &FakeRepository::add_package)

        .def("add_version", &FakeRepositoryWrapper::add_version)
        ;
}
