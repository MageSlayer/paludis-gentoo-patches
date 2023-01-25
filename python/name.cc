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
#include <python/exception.hh>
#include <python/wrapped_value.hh>
#include <python/iterable.hh>
#include <python/options.hh>
#include <paludis/util/set-impl.hh>
#include <paludis/util/wrapped_forward_iterator.hh>

#include <paludis/name.hh>
#include <paludis/slot.hh>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

void expose_name()
{
    /**
     * Exceptions
     */
    ExceptionRegister::get_instance()->add_exception<PackageNamePartError>
        ("PackageNamePartError", "NameError",
         "Thrown if an invalid value is assigned to a PackageNamePart.");
    ExceptionRegister::get_instance()->add_exception<CategoryNamePartError>
        ("CategoryNamePartError", "NameError",
         "Thrown if an invalid value is assigned to a CategoryNamePart.");
    ExceptionRegister::get_instance()->add_exception<SlotNameError>
        ("SlotNameError", "NameError",
         "Thrown if an invalid value is assigned to a SlotName.");
    ExceptionRegister::get_instance()->add_exception<RepositoryNameError>
        ("RepositoryNameError", "NameError",
         "Thrown if an invalid value is assigned to a RepositoryName.");
    ExceptionRegister::get_instance()->add_exception<KeywordNameError>
        ("KeywordNameError", "NameError",
         "Thrown if an invalid value is assigned to a KeywordName.");
    ExceptionRegister::get_instance()->add_exception<SetNameError>
        ("SetNameError", "NameError",
         "Thrown if an invalid value is assigned to a SetName.");

    /**
     * PackageNamePart
     */
    register_shared_ptrs_to_python<PackageNamePart>();
    class_wrapped_value<PackageNamePart>
        (
         "PackageNamePart",
         "Holds a string that is a valid name for the package part of a QualifiedPackageName."
        );

    /**
     * PackageNamePartIterable
     */
    class_iterable<PackageNamePartSet>
        (
         "PackageNamePartIterable",
         "Iterable of PackageNamePart"
        );

    /**
     * CategoryNamePart
     */
    register_shared_ptrs_to_python<CategoryNamePart>();
    class_wrapped_value<CategoryNamePart>
        (
         "CategoryNamePart",
         "Holds a string that is a valid name for the category part of a QualifiedPackageName."
        )
        // CategoryNamePart + PackageNamePart = QualifiedPackageName
        .def(bp::self + bp::other<PackageNamePart>())
        ;

    /**
     * CategoryNamePartIterable
     */
    class_iterable<CategoryNamePartSet>
        (
         "CategoryNamePartIterable",
         "Iterable of CategoryNamePart",
         true
        );

    /**
     * SlotName
     */
    register_shared_ptrs_to_python<SlotName>();
    class_wrapped_value<SlotName>
        (
         "SlotName",
         "Holds a string that is a valid name for a ``SLOT``."
        );

    /**
     * RepositoryName
     */
    register_shared_ptrs_to_python<RepositoryName>();
    class_wrapped_value<RepositoryName>
        (
         "RepositoryName",
         "Holds a string that is a valid name for a Repository."
        );

    /**
     * KeywordName
     */
    class_wrapped_value<KeywordName>
        (
         "KeywordName",
         "Holds a string that is a valid name for a ``KEYWORD``."
        );

    /**
     * KeywordNameCollect
     */
    class_iterable<KeywordNameSet>
        (
         "KeywordNameIterable",
         "Iterable of KeywordName",
         true
        );

    /**
     * SetName
     */
    class_wrapped_value<SetName>
        (
         "SetName",
         "Holds a string that is a valid name for a set."
        );

    /**
     * SetNameIterable
     */
    class_iterable<SetNameSet>
        (
         "SetNameIterable",
         "Iterable of SetName",
         true
        );

    /**
     * StringSetIterable
     */
    class_iterable<Set<std::string> >
        (
         "StringSetIterable",
         "Iterable of string",
         true
        );

    /**
     * QualifiedPackageName
     */
    register_shared_ptrs_to_python<QualifiedPackageName>();
    bp::implicitly_convertible<std::string, QualifiedPackageName>();
    bp::class_<QualifiedPackageName>
        (
         "QualifiedPackageName",
         "Represents a category plus package name.",
         bp::init<const std::string &>("__init__(string)")
        )
        .def(bp::init<const CategoryNamePart &, const PackageNamePart &>())

        .add_property("category",
                &QualifiedPackageName::category,
                "[ro] CategoryNamePart"
                )

        .add_property("package",
                &QualifiedPackageName::package,
                "[ro] PackageNamePart"
                )

        .def(bp::self == bp::self)
        .def(bp::self != bp::self)
        .def(bp::self <  bp::self)
        .def(bp::self <= bp::self)
        .def(bp::self >  bp::self)
        .def(bp::self >= bp::self)

        .def(bp::self_ns::str(bp::self))
        ;

    /**
     * QualifiedPackageNameIterable
     */
    class_iterable<QualifiedPackageNameSet>
        (
         "QualifiedPackageNameIterable",
         "Iterable of QualifiedPackageName",
         true
        );


    /**
     * Slot
     */
    bp::class_<Slot>
        (
         "Slot",
         "Represents a slot",
         bp::no_init
        )

        .add_property("raw_value",
                &named_values_getter<Slot, n::raw_value, std::string, &Slot::raw_value>,
                "[ro] String"
                )
        ;
}
