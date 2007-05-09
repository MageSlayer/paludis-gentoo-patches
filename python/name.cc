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

#include <paludis/name.hh>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

void PALUDIS_VISIBLE expose_name()
{
    static register_exception<PackageNamePartError>
        PackageNamePartError("PackageNamePartError");
    static register_exception<CategoryNamePartError>
        CategoryNamePartError("CategoryNamePartError");
    static register_exception<QualifiedPackageNameError>
        QualifiedPackageNameError("QualifiedPackageNameError");
    static register_exception<UseFlagNameError>
        UseFlagNameError("UseFlagNameError");
    static register_exception<SlotNameError>
        SlotNameError("SlotNameError");
    static register_exception<RepositoryNameError>
        RepositoryNameError("RepositoryNameError");
    static register_exception<KeywordNameError>
        KeywordNameError("KeywordNameError");
    static register_exception<SetNameError>
        SetNameError("SetNameError");

    class_validated<PackageNamePart>
        pnp("PackageNamePart",
                "Holds a string that is a valid name for the package part of a QualifiedPackageName."
           );
    register_shared_ptrs_to_python<PackageNamePart>();

    class_collection<PackageNamePartCollection>
        pnpc("PackageNamePartCollection",
                "Iterable collection of PackageNamePart instances."
            );

    class_validated<CategoryNamePart>
        cnp("CategoryNamePart",
                "Holds a string that is a valid name for the category part of a QualifiedPackageName."
           );
    // CategoryNamePart + PackageNamePart = QualifiedPackageName
    cnp.def(bp::self + bp::other<PackageNamePart>());
    register_shared_ptrs_to_python<CategoryNamePart>();

    class_collection<CategoryNamePartCollection>
        cnpc("CategoryNamePartCollection",
                "Iterable collection of CategoryNamePart instances."
            );

    class_validated<UseFlagName>
        ufn("UseFlagName",
                "Holds a string that is a valid name for a USE flag."
           );

    class_collection<UseFlagNameCollection>
        ufnc("UseFlagNameCollection",
                "Iterable collection of UseFlagName instances."
            );

    class_validated<SlotName>
        sln("SlotName",
                "Holds a string that is a valid name for a SLOT."
           );
    register_shared_ptrs_to_python<SlotName>();

    class_validated<RepositoryName>
        rn("RepositoryName",
                "Holds a string that is a valid name for a Repository."
          );
    register_shared_ptrs_to_python<RepositoryName>();

    class_collection<RepositoryNameCollection>
        rnc("RepositoryNameCollection",
                "Iterable collection of RepositoryName instances."
            );

    class_validated<KeywordName>
        kn("KeywordName",
                "Holds a string that is a valid name for a KEYWORD."
          );

    class_validated<SetName>
        stn("SetName",
                "Holds a string that is a valid name for a set."
           );

    class_collection<SetNameCollection>
        sc("SetNameCollection",
                "Iterable of SetName\n"
                "A collection of set names."
          );

    register_shared_ptrs_to_python<QualifiedPackageName>();
    bp::class_<QualifiedPackageName>
        qpn("QualifiedPackageName",
                "Represents a category plus package name.",
                bp::init<const std::string &>("__init__(string)")
           );
    qpn.def(bp::init<const CategoryNamePart &, const PackageNamePart &>());
    qpn.def_readwrite("category", &QualifiedPackageName::category);
    qpn.def_readwrite("package", &QualifiedPackageName::package);
    qpn.def("__cmp__", &QualifiedPackageName::compare);
    qpn.def(bp::self_ns::str(bp::self));
    bp::implicitly_convertible<std::string, QualifiedPackageName>();

    class_collection<QualifiedPackageNameCollection>
        qpnc("QualifiedPackageNameCollection",
                "Iterable collection of QualifiedPackageName instances."
            );

    bp::enum_<UseFlagState>
        ufs("UseFlagState");
    ufs.value("UNSPECIFIED", use_unspecified);
    ufs.value("DISABLED", use_disabled);
    ufs.value("ENABLED", use_enabled);
}
