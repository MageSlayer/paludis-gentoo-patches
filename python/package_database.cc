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
    ExceptionRegister::get_instance()->add_exception<PackageDatabaseError>
        ("PackageDatabaseError", "BaseException");
    ExceptionRegister::get_instance()->add_exception<DuplicateRepositoryError>
        ("DuplicateRepositoryError", "PackageDatabaseError");
    ExceptionRegister::get_instance()->add_exception<PackageDatabaseLookupError>
        ("PackageDatabaseLookupError", "PackageDatabaseError");
    ExceptionRegister::get_instance()->add_exception<AmbiguousPackageNameError>
        ("AmbiguousPackageNameError", "PackageDatabaseLookupError");
    ExceptionRegister::get_instance()->add_exception<NoSuchPackageError>
        ("NoSuchPackageError", "PackageDatabaseLookupError");
    ExceptionRegister::get_instance()->add_exception<NoSuchRepositoryError>
        ("NoSuchRepositoryError", "PackageDatabaseLookupError");

    enum_auto("QueryOrder", last_qo);

    register_shared_ptrs_to_python<PackageDatabase>();
    bp::class_<PackageDatabase, boost::noncopyable>
        pd("PackageDatabase",
                "A PackageDatabase can be queried for Package instances.\n",
                bp::no_init
          );
    tr1::shared_ptr<PackageDatabaseEntryCollection>
        (PackageDatabase::*query)(const Query &, const QueryOrder) const = &PackageDatabase::query;
    pd.def("query", query,
            "query(Query, QueryOrder) -> PackageDatabaseEntryCollection\n"
            "Query the repository."
          );
    pd.add_property("favourite_repository", &PackageDatabase::favourite_repository,
            "[ro] RepositoryName\n"
            "Name of our 'favourite' repository"
          );
    tr1::shared_ptr<const Repository>
        (PackageDatabase::* fetch_repository)(const RepositoryName &) const =
        &PackageDatabase::fetch_repository;
    pd.def("fetch_repository", fetch_repository,
            "fetch_repository(RepositoryName) -> Repository\n"
            "Fetch a named repository."
          );
    pd.def("fetch_unique_qualified_package_name", &PackageDatabase::fetch_unique_qualified_package_name,
            "fetch_unique_qualified_package_name(PackageNamePart) -> QualifiedPackageName\n"
            "Disambiguate a package name."
          );
    pd.def("more_important_than", &PackageDatabase::more_important_than,
            "more_important_than(RepositoryName, RepositoryName) -> bool\n"
            "Return true if the first repository is more important than the second."
          );
    pd.add_property("repositories",
            bp::range(&PackageDatabase::begin_repositories, &PackageDatabase::end_repositories),
            "[ro] Iterable of Repository\n"
            "Our repositories"
            );

    bp::register_ptr_to_python<tr1::shared_ptr<PackageDatabaseEntry> >();
    bp::class_<PackageDatabaseEntry>
        pde("PackageDatabaseEntry",
                "Holds an entry in a PackageDatabase, and used to identify"
                " a specific version of a package in a particular repository.",
                bp::init<const QualifiedPackageName &, const VersionSpec &, const RepositoryName &>(
                    "__init__(QualifiedPackageName, VersionSpec, RepositoryName)"
                    )
           );
    pde.def(bp::self_ns::str(bp::self));
    pde.def("__eq__", &PackageDatabaseEntry::operator==);
    pde.def("__ne__", &__ne__<PackageDatabaseEntry>);

    class_collection<PackageDatabaseEntryCollection>
        pdec("PackageDatabaseEntryCollection",
                "An iterable collection of PackageDatabaseEntry instances."
            );
}
