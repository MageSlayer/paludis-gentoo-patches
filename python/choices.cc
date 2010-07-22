/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2010 Ciaran McCreesh
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

#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/choice.hh>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

void expose_choices()
{
    bp::class_<Choices, std::shared_ptr<const Choices>, boost::noncopyable> choices(
            "Choices",
            "A collection of configurable values for a PackageID",
            bp::init<>("__init__()")
            );

    choices
        .def("find_by_name_with_prefix", &Choices::find_by_name_with_prefix,
                "find_by_name_with_prefix(ChoiceNameWithPrefix) -> ChoiceValue or nil\n"
                )

        .def("has_matching_contains_every_value_prefix", &Choices::has_matching_contains_every_value_prefix,
                "has_matching_contains_every_value_prefix(ChoiceNameWithPrefix) -> bool\n"
                )

        .def("__iter__", bp::range(&Choices::begin, &Choices::end),
                "[ro] Iterable of Choice\n"
                )
        ;

    bp::class_<Choice, std::shared_ptr<const Choice>, boost::noncopyable> choice(
            "Choice",
            "An individual choice in a Choices collection.",
            bp::no_init
            );

    choice
        .add_property("raw_name", &Choice::raw_name,
                "[ro] String\n"
                )

        .add_property("human_name", &Choice::human_name,
                "[ro] String\n"
                )

        .add_property("prefix", &Choice::prefix,
                "[ro] ChoicePrefixName\n"
                )

        .add_property("contains_every_value", &Choice::contains_every_value,
                "[ro] bool\n"
                )

        .add_property("hidden", &Choice::hidden,
                "[ro] bool\n"
                )

        .add_property("show_with_no_prefix", &Choice::show_with_no_prefix,
                "[ro] bool\n"
                )

        .add_property("consider_added_or_changed", &Choice::consider_added_or_changed,
                "[ro] bool\n"
                )

        .def("__iter__", bp::range(&Choice::begin, &Choice::end),
                "[ro] Iterable of ChoiceValue\n"
                )
        ;

    bp::class_<ChoiceValue, std::shared_ptr<const ChoiceValue>, boost::noncopyable> choice_value(
            "ChoiceValue",
            "An individual value in a ChoiceValue",
            bp::no_init
            );

    choice_value
        .add_property("unprefixed_name", &ChoiceValue::unprefixed_name,
                "[ro] UnprefixedChoiceName\n"
                )

        .add_property("name_with_prefix", &ChoiceValue::name_with_prefix,
                "[ro] ChoiceNameWithPrefix\n"
                )

        .add_property("enabled", &ChoiceValue::enabled,
                "[ro] bool\n"
                )

        .add_property("enabled_by_default", &ChoiceValue::enabled_by_default,
                "[ro] bool\n"
                )

        .add_property("locked", &ChoiceValue::locked,
                "[ro] bool\n"
                )

        .add_property("description", &ChoiceValue::description,
                "[ro] String\n"
                )

        .add_property("explicitly_listed", &ChoiceValue::explicitly_listed,
                "[ro] bool\n"
                )

        ;

    ExceptionRegister::get_instance()->add_exception<ChoiceNameWithPrefixError>
        ("ChoiceNameWithPrefixError", "NameError",
         "Thrown if an invalid value is assigned to a ChoiceNameWithPrefix.");

    register_shared_ptrs_to_python<ChoiceNameWithPrefix>();
    class_wrapped_value<ChoiceNameWithPrefix>
        (
         "ChoiceNameWithPrefix",
         "A choice name, including prefix and delim."
        );

    ExceptionRegister::get_instance()->add_exception<ChoicePrefixNameError>
        ("ChoicePrefixNameError", "NameError",
         "Thrown if an invalid value is assigned to a ChoicePrefixName.");

    register_shared_ptrs_to_python<ChoicePrefixName>();
    class_wrapped_value<ChoicePrefixName>
        (
         "ChoicePrefixName",
         "A choice prefix name."
        );

    ExceptionRegister::get_instance()->add_exception<UnprefixedChoiceNameError>
        ("UnprefixedChoiceNameError", "NameError",
         "Thrown if an invalid value is assigned to an UnprefixedChoiceName.");

    register_shared_ptrs_to_python<UnprefixedChoiceName>();
    class_wrapped_value<UnprefixedChoiceName>
        (
         "UnprefixedChoiceName",
         "A choice name, without prefix."
        );
}

