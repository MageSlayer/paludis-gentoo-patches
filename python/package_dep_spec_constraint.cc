/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Piotr Jaroszyński
 * Copyright (c) 2011 Ciaran McCreesh
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
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/fs_path.hh>
#include <paludis/package_dep_spec_constraint.hh>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

// For classes derived from PackageDepSpecConstraint
template <typename C_>
class class_package_dep_spec_constraint :
    public bp::class_<C_, std::shared_ptr<C_>, bp::bases<PackageDepSpecConstraint>, boost::noncopyable>
{
    public:
        template <class Init_>
        class_package_dep_spec_constraint(const std::string & name, const std::string & class_doc, Init_ initspec) :
            bp::class_<C_, std::shared_ptr<C_>, bp::bases<PackageDepSpecConstraint>, boost::noncopyable>(
                    name.c_str(), class_doc.c_str(), initspec)
        {
            bp::register_ptr_to_python<std::shared_ptr<const C_> >();
            bp::implicitly_convertible<std::shared_ptr<C_>, std::shared_ptr<PackageDepSpecConstraint> >();
        }
};

void expose_package_dep_spec_constraint()
{
    /**
     * PackageDepSpecConstraint
     */
    bp::register_ptr_to_python<std::shared_ptr<const PackageDepSpecConstraint> >();
    bp::implicitly_convertible<std::shared_ptr<PackageDepSpecConstraint>,
            std::shared_ptr<const PackageDepSpecConstraint> >();
    bp::class_<PackageDepSpecConstraint, boost::noncopyable>
        (
         "PackageDepSpecConstraint",
         "Base class for a constraint for a PackageDepSpec.",
         bp::no_init
        )
        ;

    /**
     * NameConstraint
     */
    class_package_dep_spec_constraint<NameConstraint>
        (
         "NameConstraint",
         "A cat/pkg constraint for a PackageDepSpec.",
         bp::no_init
        )

        .add_property("name", &NameConstraint::name,
                "[RO] The cat/pkg in question"
            )
        ;

    /**
     * PackageNamePartConstraint
     */
    class_package_dep_spec_constraint<PackageNamePartConstraint>
        (
         "PackageNamePartConstraint",
         "A /pkg constraint for a PackageDepSpec.",
         bp::no_init
        )

        .add_property("name_part", &PackageNamePartConstraint::name_part,
                "[RO] The /pkg in question"
            )
        ;

    /**
     * CategoryNamePartConstraint
     */
    class_package_dep_spec_constraint<CategoryNamePartConstraint>
        (
         "CategoryNamePartConstraint",
         "A cat/ constraint for a PackageDepSpec.",
         bp::no_init
        )

        .add_property("name_part", &CategoryNamePartConstraint::name_part,
                "[RO] The cat/ in question"
            )
        ;

    /**
     * InRepositoryConstraint
     */
    class_package_dep_spec_constraint<InRepositoryConstraint>
        (
         "InRepositoryConstraint",
         "A ::repo constraint for a PackageDepSpec.",
         bp::no_init
        )

        .add_property("name", &InRepositoryConstraint::name,
                "[RO] The ::repo name in question"
            )
        ;
}

