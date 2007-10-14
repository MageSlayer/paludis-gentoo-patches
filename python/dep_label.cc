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

#include <python/paludis_python.hh>
#include <python/exception.hh>
#include <python/iterable.hh>

#include <paludis/dep_label.hh>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

template <typename L_>
struct class_concrete_uri_label :
    bp::class_<L_, tr1::shared_ptr<L_>, bp::bases<URILabel>, boost::noncopyable>
{
    class_concrete_uri_label(const std::string & name) :
        bp::class_<L_, tr1::shared_ptr<L_>, bp::bases<URILabel>, boost::noncopyable>
        (
         name.c_str(),
         "A concrete URI label class.",
         bp::init<const std::string &>("__init__(string)")
        )
    {
        bp::implicitly_convertible<tr1::shared_ptr<L_>, tr1::shared_ptr<URILabel> >();
    }
};

template <typename L_, typename C_>
struct class_concrete_dependency_label :
    bp::class_<L_, tr1::shared_ptr<L_>, bp::bases<C_>, boost::noncopyable>
{
    class_concrete_dependency_label(const std::string & name, const std::string & doc) :
        bp::class_<L_, tr1::shared_ptr<L_>, bp::bases<C_>, boost::noncopyable>
        (
         name.c_str(),
         doc.c_str(),
         bp::init<const std::string &>("__init__(string)")
        )
    {
        bp::implicitly_convertible<tr1::shared_ptr<L_>, tr1::shared_ptr<C_> >();
    }
};

void expose_dep_label()
{
    /**
     * URILabel
     */
    register_shared_ptrs_to_python<URILabel>();
    bp::class_<URILabel, boost::noncopyable>
        (
         "URILabel",
         "URI label base class.",
         bp::no_init
        )
        .add_property("text", &URILabel::text,
                "[ro] string\n"
                "Our text."
                )

        .def("__str__", &URILabel::text)
        ;

    /**
     * ConcreteURILabels
     */
    class_concrete_uri_label<URIMirrorsThenListedLabel>("URIMirrorsThenListedLabel");
    class_concrete_uri_label<URIMirrorsOnlyLabel>("URIMirrorsOnlyLabel");
    class_concrete_uri_label<URIListedOnlyLabel>("URIListedOnlyLabel");
    class_concrete_uri_label<URIListedThenMirrorsLabel>("URIListedThenMirrorsLabel");
    class_concrete_uri_label<URILocalMirrorsOnlyLabel>("URILocalMirrorsOnlyLabel");
    class_concrete_uri_label<URIManualOnlyLabel>("URIManualOnlyLabel");

    /**
     * DependencyLabel
     */
    register_shared_ptrs_to_python<DependencyLabel>();
    bp::class_<DependencyLabel, boost::noncopyable>
        (
         "DependencyLabel",
         "Dependency label base class.",
         bp::no_init
        )
        .add_property("text", &DependencyLabel::text,
                "[ro] string\n"
                "Our text."
                )

        .def("__str__", &DependencyLabel::text)
        ;

    /**
     * DependencySystemLabel
     */
    bp::implicitly_convertible<tr1::shared_ptr<DependencySystemLabel>, tr1::shared_ptr<DependencyLabel> >();
    register_shared_ptrs_to_python<DependencySystemLabel>();
    bp::class_<DependencySystemLabel, bp::bases<DependencyLabel>, boost::noncopyable>
        (
         "DependencySystemLabel",
         "System dependency label base class.",
         bp::no_init
        );

    /**
     * DependencyTypeLabel
     */
    bp::implicitly_convertible<tr1::shared_ptr<DependencyTypeLabel>, tr1::shared_ptr<DependencyLabel> >();
    register_shared_ptrs_to_python<DependencyTypeLabel>();
    bp::class_<DependencyTypeLabel, bp::bases<DependencyLabel>, boost::noncopyable>
        (
         "DependencyTypeLabel",
         "Type dependency label base class.",
         bp::no_init
        );

    /**
     * DependencySuggestLabel
     */
    bp::implicitly_convertible<tr1::shared_ptr<DependencySuggestLabel>, tr1::shared_ptr<DependencyLabel> >();
    register_shared_ptrs_to_python<DependencySuggestLabel>();
    bp::class_<DependencySuggestLabel, bp::bases<DependencyLabel>, boost::noncopyable>
        (
         "DependencySuggestLabel",
         "Suggest dependency label base class.",
         bp::no_init
        );

    /**
     * DependencyABIsLabel
     */
    bp::implicitly_convertible<tr1::shared_ptr<DependencyABIsLabel>, tr1::shared_ptr<DependencyLabel> >();
    register_shared_ptrs_to_python<DependencyABIsLabel>();
    bp::class_<DependencyABIsLabel, bp::bases<DependencyLabel>, boost::noncopyable>
        (
         "DependencyABIsLabel",
         "ABI dependency label base class.",
         bp::no_init
        );

    /**
     * ConcreteDependencyLabels
     */
    class_concrete_dependency_label<DependencyHostLabel, DependencySystemLabel>
        ("DependencyHostLabel",
         "A DependencyHostLabel specifies host requirements for building a package.");

    class_concrete_dependency_label<DependencyTargetLabel, DependencySystemLabel>
        ("DependencyTargetLabel",
         "A DependencyTargetLabel specifies target requirements for building a package.");

    class_concrete_dependency_label<DependencyBuildLabel, DependencyTypeLabel>
        ("DependencyBuildLabel",
         "A DependencyBuildLabel specifies build-time requirements for building a package.");

    class_concrete_dependency_label<DependencyRunLabel, DependencyTypeLabel>
        ("DependencyRunLabel",
         "A DependencyRunLabel specifies runtime requirements for building a package.");

    class_concrete_dependency_label<DependencyInstallLabel, DependencyTypeLabel>
        ("DependencyInstallLabel",
         "A DependencyInstallLabel specifies install-time requirements for building a package.");

    class_concrete_dependency_label<DependencyCompileLabel, DependencyTypeLabel>
        ("DependencyCompileLabel",
         "A DependencyCompileLabel specifies compiled-against requirements for building a package.");

    class_concrete_dependency_label<DependencySuggestedLabel, DependencySuggestLabel>
        ("DependencySuggestedLabel",
         "A DependencySuggestLabel specifies that a dependency is suggested.");

    class_concrete_dependency_label<DependencyRecommendedLabel, DependencySuggestLabel>
        ("DependencyRecommendedLabel",
         "A DependencyRecommendedLabel specifies that a dependency is recommended.");

    class_concrete_dependency_label<DependencyRequiredLabel, DependencySuggestLabel>
        ("DependencyRequiredLabel",
         "* A DependencyRequiredLabel specifies that a dependency is required.");

    class_concrete_dependency_label<DependencyAnyLabel, DependencyABIsLabel>
        ("DependencyAnyLabel",
         "A DependencyAnyLabel specifies that a dependency can be satisfied by\n"
         "any ABI.");

    class_concrete_dependency_label<DependencyMineLabel, DependencyABIsLabel>
        ("DependencyMineLabel",
         "A DependencyMineLabel specifies that a dependency is satisfied by\n"
         "ABIs equal to those being used to create the depending package.");

    class_concrete_dependency_label<DependencyPrimaryLabel, DependencyABIsLabel>
        ("DependencyPrimaryLabel",
         "A DependencyPrimaryLabel specifies that a dependency can be satisfied by\n"
         "the primary ABI.");

    class_concrete_dependency_label<DependencyABILabel, DependencyABIsLabel>
        ("DependencyABILabel",
         "A DependencyABILabel specifies that a dependency can be satisfied by\n"
         "a named ABI.");
}
