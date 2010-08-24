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

#include <paludis/filtered_generator.hh>
#include <paludis/filter.hh>
#include <paludis/generator.hh>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

void expose_filtered_generator()
{
    bp::class_<FilteredGenerator> filtered_generator(
            "FilteredGenerator",
            "A combined Generator and Filter for an Environment selection.",
            bp::init<const Generator &, const Filter &>("__init__(generator, filter)")
            );

    filtered_generator
        .def("filter", bp::make_function(&FilteredGenerator::filter,
                    bp::return_value_policy<bp::copy_const_reference>()),
                "Our Filter."
            )
        .def("generator", bp::make_function(&FilteredGenerator::generator,
                    bp::return_value_policy<bp::copy_const_reference>()),
                "Our Generator."
            )
        .def("__or__", static_cast<FilteredGenerator (*) (const FilteredGenerator &, const Filter &)>(
                    &paludis::operator| ),
                "Add a new Filter."
            )
        .def(bp::self_ns::str(bp::self))
        ;
}

