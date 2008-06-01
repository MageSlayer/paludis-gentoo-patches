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
#include <paludis/dep_spec.hh>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

void expose_generator()
{
    bp::class_<Generator> generator(
            "Generator",
            "Generator for an Environment selection.",
            bp::no_init
            );

    generator
        .def(bp::self_ns::str(bp::self))
        .def("__or__", static_cast<FilteredGenerator (*) (const FilteredGenerator &, const Filter &)>(
                    &paludis::operator| ),
                "Combine with a Filter to produce a FilteredGenerator."
            )
        .def("__and__", static_cast<Generator (*) (const Generator &, const Generator &)>(
                    &paludis::operator& ),
                "Combine with another Generator using a set union."
            )
        ;

    bp::implicitly_convertible<Generator, FilteredGenerator>();

    bp::scope generator_scope = generator;

    bp::class_<generator::All, bp::bases<Generator> > generator_all(
            "All",
            "Generate all packages.",
            bp::init<>("__init__()")
            );

    bp::class_<generator::Matches, bp::bases<Generator> > generator_matches(
            "Matches",
            "Generate matching packages.",
            bp::init<const PackageDepSpec &>("__init__(spec)")
            );

    bp::class_<generator::Union, bp::bases<Generator> > generator_union(
            "Union",
            "Generate packages from a union of two other Generator instances.",
            bp::init<const Generator &, const Generator &>("__init__(generator, generator)")
            );

    bp::class_<generator::Package, bp::bases<Generator> > generator_package(
            "Package",
            "Generate all named packages.",
            bp::init<const QualifiedPackageName &>("__init__(name)")
            );

    bp::class_<generator::Category, bp::bases<Generator> > generator_category(
            "Category",
            "Generate all packages in a given category.",
            bp::init<const CategoryNamePart &>("__init__(category)")
            );

    bp::class_<generator::Repository, bp::bases<Generator> > generator_repository(
            "Repository",
            "Generate all packages in a given repository.",
            bp::init<const RepositoryName &>("__init__(repository)")
            );
}

