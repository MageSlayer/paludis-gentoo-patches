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
#include <paludis/package_dep_spec_requirement.hh>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

// For classes derived from PackageDepSpecRequirement
template <typename C_>
class class_package_dep_spec_requirement :
    public bp::class_<C_, std::shared_ptr<C_>, bp::bases<PackageDepSpecRequirement>, boost::noncopyable>
{
    public:
        template <class Init_>
        class_package_dep_spec_requirement(const std::string & name, const std::string & class_doc, Init_ initspec) :
            bp::class_<C_, std::shared_ptr<C_>, bp::bases<PackageDepSpecRequirement>, boost::noncopyable>(
                    name.c_str(), class_doc.c_str(), initspec)
        {
            bp::register_ptr_to_python<std::shared_ptr<const C_> >();
            bp::implicitly_convertible<std::shared_ptr<C_>, std::shared_ptr<PackageDepSpecRequirement> >();
        }
};

void expose_package_dep_spec_requirement()
{
    /**
     * Enums
     */
    enum_auto("KeyRequirementOperation", last_kro,
            "The operation for a KeyRequirement");

    /**
     * PackageDepSpecRequirement
     */
    bp::register_ptr_to_python<std::shared_ptr<const PackageDepSpecRequirement> >();
    bp::implicitly_convertible<std::shared_ptr<PackageDepSpecRequirement>,
            std::shared_ptr<const PackageDepSpecRequirement> >();
    bp::class_<PackageDepSpecRequirement, boost::noncopyable>
        (
         "PackageDepSpecRequirement",
         "Base class for a requirement for a PackageDepSpec.",
         bp::no_init
        )
        ;

    /**
     * NameRequirement
     */
    class_package_dep_spec_requirement<NameRequirement>
        (
         "NameRequirement",
         "A cat/pkg requirement for a PackageDepSpec.",
         bp::no_init
        )

        .add_property("name", &NameRequirement::name,
                "[RO] The cat/pkg in question"
            )
        ;

    /**
     * PackageNamePartRequirement
     */
    class_package_dep_spec_requirement<PackageNamePartRequirement>
        (
         "PackageNamePartRequirement",
         "A /pkg requirement for a PackageDepSpec.",
         bp::no_init
        )

        .add_property("name_part", &PackageNamePartRequirement::name_part,
                "[RO] The /pkg in question"
            )
        ;

    /**
     * CategoryNamePartRequirement
     */
    class_package_dep_spec_requirement<CategoryNamePartRequirement>
        (
         "CategoryNamePartRequirement",
         "A cat/ requirement for a PackageDepSpec.",
         bp::no_init
        )

        .add_property("name_part", &CategoryNamePartRequirement::name_part,
                "[RO] The cat/ in question"
            )
        ;

    /**
     * InRepositoryRequirement
     */
    class_package_dep_spec_requirement<InRepositoryRequirement>
        (
         "InRepositoryRequirement",
         "A ::repo requirement for a PackageDepSpec.",
         bp::no_init
        )

        .add_property("name", &InRepositoryRequirement::name,
                "[RO] The ::repo name in question"
            )
        ;

    /**
     * FromRepositoryRequirement
     */
    class_package_dep_spec_requirement<FromRepositoryRequirement>
        (
         "FromRepositoryRequirement",
         "A ::repo-> requirement for a PackageDepSpec.",
         bp::no_init
        )

        .add_property("name", &FromRepositoryRequirement::name,
                "[RO] The ::repo-> name in question"
            )
        ;

    /**
     * InstalledAtPathRequirement
     */
    class_package_dep_spec_requirement<InstalledAtPathRequirement>
        (
         "InstalledAtPathRequirement",
         "A ::/ requirement for a PackageDepSpec.",
         bp::no_init
        )

        .add_property("path", &InstalledAtPathRequirement::path,
                "[RO] The ::/ path in question"
            )
        ;

    /**
     * InstallableToPathRequirement
     */
    class_package_dep_spec_requirement<InstallableToPathRequirement>
        (
         "InstalledableToPathRequirement",
         "A ::/? requirement for a PackageDepSpec.",
         bp::no_init
        )

        .add_property("path", &InstallableToPathRequirement::path,
                "[RO] The ::/? path in question"
            )

        .add_property("include_masked", &InstallableToPathRequirement::include_masked,
                "[RO] Whether to include masked, as per ::/??"
            )
        ;

    /**
     * InstallableToRepositoryRequirement
     */
    class_package_dep_spec_requirement<InstallableToRepositoryRequirement>
        (
         "InstalledableToPathRequirement",
         "A ::/? requirement for a PackageDepSpec.",
         bp::no_init
        )

        .add_property("name", &InstallableToRepositoryRequirement::name,
                "[RO] The ::repo? in question"
            )

        .add_property("include_masked", &InstallableToRepositoryRequirement::include_masked,
                "[RO] Whether to include masked, as per ::repo??"
            )
        ;

    /**
     * AnySlotRequirement
     */
    class_package_dep_spec_requirement<AnySlotRequirement>
        (
         "AnySlotRequirement",
         "A :* or := requirement for a PackageDepSpec.",
         bp::no_init
        )

        .add_property("locking", &AnySlotRequirement::locking,
                "[RO] Are we locking (:= rather than :*)?"
            )
        ;

    /**
     * ExactSlotRequirement
     */
    class_package_dep_spec_requirement<ExactSlotRequirement>
        (
         "ExactSlotRequirement",
         "A :slot or :=slot requirement for a PackageDepSpec.",
         bp::no_init
        )

        .add_property("locked", &ExactSlotRequirement::locked,
                "[RO] Are we locked (:=blah)?"
            )

        .add_property("name", &ExactSlotRequirement::name,
                "[RO] The slot name"
            )
        ;

    /**
     * KeyRequirement
     */
    class_package_dep_spec_requirement<KeyRequirement>
        (
         "KeyRequirement",
         "A [.key=value] requirement for a PackageDepSpec.",
         bp::no_init
        )

        .add_property("key", &KeyRequirement::key,
                "[RO] The key"
            )

        .add_property("pattern", &KeyRequirement::pattern,
                "[RO] The pattern"
            )

        .add_property("operation", &KeyRequirement::operation,
                "[RO] The operation"
            )
        ;
}

