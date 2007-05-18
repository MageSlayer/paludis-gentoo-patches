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
    static register_exception<IUseFlagNameError>
        IUseFlagNameError("IUseFlagNameError");
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
                "Iterable of PackageNamePart.\n"
                "A collection of PackageNamePart instances."
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
                "Iterable of CategoryNamePart.\n"
                "A collection of CategoryNamePart instances."
            );

    class_validated<UseFlagName>
        ufn("UseFlagName",
                "Holds a string that is a valid name for a USE flag."
           );

    class_collection<UseFlagNameCollection>
        ufnc("UseFlagNameCollection",
                "Iterable of UseFlagName.\n"
                "A collection of UseFlagName instances."
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
                "Iterable of RepositoryName\n"
                "A collection of RepositoryName instances."
            );

    class_validated<KeywordName>
        kn("KeywordName",
                "Holds a string that is a valid name for a KEYWORD."
          );

    class_collection<KeywordNameCollection>
        knc("KeywordNameCollection",
                "Iterable of KeywordName\n"
                "A collection of KeywordName instances."
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

    class_collection<InheritedCollection>
        ic("InheritedCollection",
                "Iterable of string\n"
                "A collection of inherited packages."
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
#ifdef CIARANM_REMOVED_THIS
    qpn.def("__cmp__", &QualifiedPackageName::compare);
#endif
    qpn.def(bp::self_ns::str(bp::self));
    bp::implicitly_convertible<std::string, QualifiedPackageName>();

    class_collection<QualifiedPackageNameCollection>
        qpnc("QualifiedPackageNameCollection",
                "Iterable of QualifiedPackageName\n"
                "A collection of QualifiedPackageName instances."
            );

    bp::class_<IUseFlag>
        iuf("IUseFlag",
                "Represents an IUse flag.",
                bp::init<const std::string &, IUseFlagParseMode>("__init__(string, IUseFlagParseMode")
           );
    iuf.def(bp::init<const UseFlagName &, const UseFlagState &>("__init__(UseFlagName, UseFlagState)"));
#ifdef CIARANM_REMOVED_THIS
    iuf.def("__cmp__", &IUseFlag::compare);
#endif
    iuf.def(bp::self_ns::str(bp::self));
    iuf.def_readwrite("flag", &IUseFlag::flag,
            "[rw] UseFlagName"
            );
    iuf.def_readwrite("state", &IUseFlag::state,
            "[rw] UseFlagState"
            );

    class_collection<IUseFlagCollection>
        iufc("IUseFlagCollection",
                "Iterable of IUseFlag\n"
                "A collection of use flags."
            );

    enum_auto("UseFlagState", last_use);
    enum_auto("IUseFlagParseMode", last_iuse_pm);
}
