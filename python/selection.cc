/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Ciaran McCreesh
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

#include <paludis/generator.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/filter.hh>
#include <paludis/selection.hh>
#include <paludis/dep_spec.hh>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

void expose_selection()
{
    bp::class_<Selection> selection(
            "Selection",
            "Selection for an Environment selection.",
            bp::no_init
            );

    selection
        .def(bp::self_ns::str(bp::self))
        ;

    bp::scope selection_scope = selection;

    bp::class_<selection::SomeArbitraryVersion, bp::bases<Selection> > selection_some_arbitrary_version(
            "SomeArbitraryVersion",
            "Select some arbitrary version of some arbitrary package.",
            bp::init<const FilteredGenerator &>("__init__(filtered_generator)")
            );

    bp::class_<selection::BestVersionOnly, bp::bases<Selection> > selection_best_version_only(
            "BestVersionOnly",
            "Select the best version only of each package.",
            bp::init<const FilteredGenerator &>("__init__(filtered_generator)")
            );

    bp::class_<selection::BestVersionInEachSlot, bp::bases<Selection> > selection_best_version_in_each_slot(
            "BestVersionInEachSlot",
            "Select the best version in each slot of each package.",
            bp::init<const FilteredGenerator &>("__init__(filtered_generator)")
            );

    bp::class_<selection::AllVersionsSorted, bp::bases<Selection> > selection_all_versions_sorted(
            "AllVersionsSorted",
            "Select all versions, sorted.",
            bp::init<const FilteredGenerator &>("__init__(filtered_generator)")
            );

    bp::class_<selection::AllVersionsGroupedBySlot, bp::bases<Selection> > selection_all_versions_grouped_by_slot(
            "AllVersionsGroupedBySlot",
            "Select all versions, sorted and grouped by slot.",
            bp::init<const FilteredGenerator &>("__init__(filtered_generator)")
            );

    bp::class_<selection::AllVersionsUnsorted, bp::bases<Selection> > selection_all_versions_unsorted(
            "AllVersionsUnsorted",
            "Select all versions, in no particular order.",
            bp::init<const FilteredGenerator &>("__init__(filtered_generator)")
            );

    bp::class_<selection::RequireExactlyOne, bp::bases<Selection> > selection_require_exactly_one(
            "RequireExactlyOne",
            "Require exactly one matching ID.",
            bp::init<const FilteredGenerator &>("__init__(filtered_generator)")
            );
}

