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

#include <paludis/dep_spec.hh>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

void PALUDIS_VISIBLE expose_dep_spec()
{
    static register_exception<PackageDepSpecError>
        PackageDepSpecError("PackageDepSpecError");

    enum_auto("PackageDepSpecParseMode", last_pds_pm);

    register_shared_ptrs_to_python<DepSpec>();
    bp::class_<DepSpec, boost::noncopyable>
        ds("DepSpec",
                "Base class for a dependency spec.",
                bp::no_init
          );
    ds.def("as_use_dep_spec", &DepSpec::as_use_dep_spec,
            bp::return_value_policy<bp::reference_existing_object>(),
            "as_use_dep_spec() -> UseDepSpec\n"
            "Return us as a UseDepSpec, or None if we are not a UseDepSpec."
          );
    ds.def("as_package_dep_spec", &DepSpec::as_package_dep_spec,
            bp::return_value_policy<bp::reference_existing_object>(),
            "as_package_dep_spec() -> PackageDepSpec\n"
            "Return us as a PackageDepSpec, or None if we are not a UseDepSpec."
          );

    register_shared_ptrs_to_python<CompositeDepSpec>();
    bp::class_<CompositeDepSpec, bp::bases<DepSpec>, boost::noncopyable>
        cds("CompositeDepSpec",
                "Iterable class for dependency specs that have a number of child dependency specs.",
                bp::no_init
          );
    cds.def("__iter__", bp::range(&CompositeDepSpec::begin, &CompositeDepSpec::end));

    bp::class_<AnyDepSpec, bp::bases<CompositeDepSpec>, boost::noncopyable>
        anyds("AnyDepSpec",
                "Represents a \"|| ( )\" dependency block.",
                bp::no_init
          );

    bp::class_<AllDepSpec, bp::bases<CompositeDepSpec>, boost::noncopyable>
        allds("AllDepSpec",
                "Represents a ( first second third ) or top level group of dependency specs.",
                bp::no_init
          );

    bp::class_<UseDepSpec, bp::bases<CompositeDepSpec>, boost::noncopyable>
        useds("UseDepSpec",
                "Represents a use? ( ) dependency spec.",
                bp::no_init
          );
    useds.add_property("flag", &UseDepSpec::flag,
            "[ro] UseFlagName\n"
            "Our use flag name."
            );
    useds.add_property("inverse", &UseDepSpec::inverse,
            "[ro] bool\n"
            "Are we a ! flag?"
            );

    bp::class_<StringDepSpec, bp::bases<DepSpec>, boost::noncopyable>
        strds("StringDepSpec",
                "A StringDepSpec represents a non-composite dep spec with an associated piece of text.",
                bp::no_init
             );

    bp::to_python_converter<std::pair<const UseFlagName, UseFlagState>,
            pair_to_tuple<const UseFlagName, UseFlagState> >();
    bp::class_<UseRequirements>
        ur("UseRequirements",
                "A selection of USE flag requirements.",
                bp::no_init
          );
    ur.def("state", &UseRequirements::state,
            "state(UseFlagName) -> UseFlagState\n"
            "What state is desired for a particular use flag?"
          );
    ur.def("__iter__", bp::range(&UseRequirements::begin, &UseRequirements::end));
    register_shared_ptrs_to_python<UseRequirements>();

    bp::class_<PackageDepSpec, std::tr1::shared_ptr<const PackageDepSpec>, bp::bases<StringDepSpec> >
        pkgds("PackageDepSpec",
                "A PackageDepSpec represents a package name (for example, 'app-editors/vim'),"
                " possibly with associated version and SLOT restrictions.",
                bp::init<const std::string &, const PackageDepSpecParseMode>(
                    "__init__(string, PackageDepSpecParseMode)"
                    )
           );
    pkgds.add_property("package", &PackageDepSpec::package_ptr,
            "[ro] QualifiedPackageName\n"
            "Qualified package name."
           );
    pkgds.add_property("package_name_part", &PackageDepSpec::package_name_part_ptr,
            "[ro] PackageNamePart\n"
            "Package name part (may be None)"
           );
    pkgds.add_property("category_name_part", &PackageDepSpec::category_name_part_ptr,
            "[ro] CategoryNamePart\n"
            "Category name part (may be None)."
           );
    std::tr1::shared_ptr<const VersionRequirements> (PackageDepSpec::*version_requirements)() const =
        &PackageDepSpec::version_requirements_ptr;
    pkgds.add_property("version_requirements", version_requirements,
            "[ro] VersionRequirements\n"
            "Version requirements (may be None)."
           );
    pkgds.add_property("version_requirements_mode", &PackageDepSpec::version_requirements_mode,
            "[ro] VersionRequirementsMode\n"
            "Version requirements mode."
           );
    pkgds.add_property("slot", &PackageDepSpec::slot_ptr,
            "[ro] SlotName\n"
            "Slot name (may be None)."
           );
    pkgds.add_property("repository", &PackageDepSpec::repository_ptr,
            "[ro] RepositoryName\n"
            "Repository name (may be None)."
           );
    pkgds.add_property("use_requirements", &PackageDepSpec::use_requirements_ptr,
            "[ro] UseRequirements\n"
            "Use requirements (may be None)."
           );
    pkgds.def(bp::self_ns::str(bp::self));

    bp::class_<PlainTextDepSpec, bp::bases<StringDepSpec>, boost::noncopyable >
        ptds("PlainTextDepSpec",
                "A PlainTextDepSpec represents a plain text entry (for example, a URI in SRC_URI).",
                bp::init<const std::string &>("__init__(string)")
           );
    ptds.def(bp::self_ns::str(bp::self));

    bp::class_<BlockDepSpec, bp::bases<StringDepSpec>, boost::noncopyable >
        bds("BlockDepSpec",
                "A BlockDepSpec represents a block on a package name (for example, 'app-editors/vim'),"
                "possibly with associated version and SLOT restrictions.",
                bp::init<std::tr1::shared_ptr<const PackageDepSpec> >("__init__(PackageDepSpec)")
           );
    bds.add_property("blocked_spec", &BlockDepSpec::blocked_spec,
            "[ro] PackageDepSpec\n"
            "The spec we're blocking."
           );
}
