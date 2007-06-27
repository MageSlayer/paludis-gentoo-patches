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

#include <paludis/dep_spec.hh>
#include <paludis/query.hh>
#include <paludis/environment.hh>
#include <paludis/package_database_entry.hh>
#include <paludis/package_database.hh>
#include <paludis/util/collection.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

void PALUDIS_VISIBLE expose_package_database()
{
    /**
     * Exceptions
     */
    ExceptionRegister::get_instance()->add_exception<PackageDatabaseError>
        ("PackageDatabaseError", "BaseException",
         "A PackageDatabaseError is an error that occurs when performing some operation upon a PackageDatabase.");
    ExceptionRegister::get_instance()->add_exception<DuplicateRepositoryError>
        ("DuplicateRepositoryError", "PackageDatabaseError",
         "Thrown if a Repository with the same name as an existing member is added to a PackageDatabase.");
    ExceptionRegister::get_instance()->add_exception<PackageDatabaseLookupError>
        ("PackageDatabaseLookupError", "PackageDatabaseError",
         "A PackageDatabaseLookupError descendent is thrown if an error occurs "
         "when looking for something in a PackageDatabase.");
    ExceptionRegister::get_instance()->add_exception<AmbiguousPackageNameError>
        ("AmbiguousPackageNameError", "PackageDatabaseLookupError",
         "Thrown if a PackageDatabase query results in more than one matching Package.");
    ExceptionRegister::get_instance()->add_exception<NoSuchPackageError>
        ("NoSuchPackageError", "PackageDatabaseLookupError",
         "Thrown if there is no Package in a PackageDatabase with the given name.");
    ExceptionRegister::get_instance()->add_exception<NoSuchRepositoryError>
        ("NoSuchRepositoryError", "PackageDatabaseLookupError",
         "Thrown if there is no Repository in a RepositoryDatabase with the given name.");

    /**
     * Enums
     */
    enum_auto("QueryOrder", last_qo,
            "How to order query results.");

    /**
     * PackageDatabase
     */
    register_shared_ptrs_to_python<PackageDatabase>();
    tr1::shared_ptr<PackageDatabaseEntryCollection>
        (PackageDatabase::* query)(const Query &, const QueryOrder) const = &PackageDatabase::query;
    tr1::shared_ptr<const Repository>
        (PackageDatabase::* fetch_repository)(const RepositoryName &) const = &PackageDatabase::fetch_repository;
    bp::class_<PackageDatabase, boost::noncopyable>
        (
         "PackageDatabase",
         "A PackageDatabase can be queried for Package instances.\n",
         bp::no_init
        )
        .def("query", query,
                "query(Query, QueryOrder) -> PackageDatabaseEntryCollection\n"
                "Query the repository."
            )

        .add_property("favourite_repository", &PackageDatabase::favourite_repository,
                "[ro] RepositoryName\n"
                "Name of our 'favourite' repository"
                )

        .def("fetch_repository", fetch_repository,
                "fetch_repository(RepositoryName) -> Repository\n"
                "Fetch a named repository."
            )

        .def("fetch_unique_qualified_package_name", &PackageDatabase::fetch_unique_qualified_package_name,
                "fetch_unique_qualified_package_name(PackageNamePart) -> QualifiedPackageName\n"
                "Disambiguate a package name."
            )

        .def("more_important_than", &PackageDatabase::more_important_than,
                "more_important_than(RepositoryName, RepositoryName) -> bool\n"
                "Return true if the first repository is more important than the second."
            )

        .add_property("repositories",
                bp::range(&PackageDatabase::begin_repositories, &PackageDatabase::end_repositories),
                "[ro] Iterable of Repository\n"
                "Our repositories"
                )
        ;

    /**
     * PackageDatabaseEntry
     */
    bp::register_ptr_to_python<tr1::shared_ptr<PackageDatabaseEntry> >();
    bp::class_<PackageDatabaseEntry>
        (
         "PackageDatabaseEntry",
         "Holds an entry in a PackageDatabase, and used to identify"
         " a specific version of a package in a particular repository.",
         bp::init<const QualifiedPackageName &, const VersionSpec &, const RepositoryName &>(
             "__init__(QualifiedPackageName, VersionSpec, RepositoryName)"
             )
        )
        .def(bp::self_ns::str(bp::self))

        .def("__eq__", &PackageDatabaseEntry::operator==)

        .def("__ne__", &py_ne<PackageDatabaseEntry>)
        ;

    /**
     * PackageDatabaseEntryCollection
     */
    class_collection<PackageDatabaseEntryCollection>
        (
         "PackageDatabaseEntryCollection",
         "An iterable collection of PackageDatabaseEntry instances."
        );
}
