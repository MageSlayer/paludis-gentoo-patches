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

#include <paludis/query.hh>
#include <paludis/dep_spec.hh>
#include <paludis/util/fs_entry.hh>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

void PALUDIS_VISIBLE expose_query()
{
    /**
     * Query
     */
    bp::class_<Query>
        q(
         "Query",
         "Parameter for a PackageDatabase query.",
         bp::no_init
        );
        q.def("__and__", operator&);

    /* I need to think about it yet... */
    bp::scope query = q;

    /**
     * Matches
     */
    bp::class_<query::Matches, bp::bases<Query> >
        (
         "Matches",
         "Fetch packages matching a given PackageDepSpec.",
         bp::init<const PackageDepSpec &>("__init__(PackageDepSpec)")
        );

    /**
     * Package
     */
    bp::class_<query::Package, bp::bases<Query> >
        (
         "Package",
         "Fetch packages with a given package name.",
         bp::init<const QualifiedPackageName &>("__init__(QualifiedPackageName)")
        );

    /**
     * Repository
     */
    bp::class_<query::Repository, bp::bases<Query> >
        (
         "Repository",
         "Fetch packages in a given repository.",
         bp::init<const RepositoryName &>("__init__(RepositoryName)")
        );

    /**
     * Category
     */
    bp::class_<query::Category, bp::bases<Query> >
        (
         "Category",
         "Fetch packages in a given Category.",
         bp::init<const CategoryNamePart &>("__init__(CategoryNamePart)")
        );

    /**
     * NotMasked
     */
    bp::class_<query::NotMasked, bp::bases<Query> >
        (
         "NotMasked",
         "Fetch packages that are not masked.",
         bp::init<>("__init__()")
        );

    /**
     * RepositoryHasInstalledInterface
     */
    bp::class_<query::RepositoryHasInstalledInterface, bp::bases<Query> >
        (
         "RepositoryHasInstalledInterface",
         "Fetch packages from a repository that has RepositoryInstalledInterface.",
         bp::init<>("__init__()")
        );

    /**
     * RepositoryHasInstallableInterface
     */
    bp::class_<query::RepositoryHasInstallableInterface, bp::bases<Query> >
        (
         "RepositoryHasInstallableInterface",
         "Fetch packages from a repository that has RepositoryInstallableInterface.",
         bp::init<>("__init__()")
        );

    /**
     * RepositoryHasUninstallableInterface
     */
    bp::class_<query::RepositoryHasUninstallableInterface, bp::bases<Query> >
        (
         "RepositoryHasUninstallableInterface",
         "Fetch packages from a repository that has RepositoryUninstallableInterface.",
         bp::init<>("__init__()")
        );

    /**
     * InstalledAtRoot
     */
    bp::class_<query::InstalledAtRoot, bp::bases<Query> >
        (
         "InstalledAtRoot",
         "Fetch packages that are installed at a particular root.",
         bp::init<const FSEntry &>("__init__(path)")
        );

    /**
     * All
     */
    bp::class_<query::All, bp::bases<Query> >
        (
         "All",
         "Fetch all packages.",
         bp::init<>("__init__()")
        );
}
