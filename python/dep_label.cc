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
#include <python/iterable.hh>

#include <paludis/dep_label.hh>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

template <typename L_>
struct class_concrete_uri_label :
    bp::class_<L_, std::tr1::shared_ptr<L_>, bp::bases<URILabel>, boost::noncopyable>
{
    class_concrete_uri_label(const std::string & name) :
        bp::class_<L_, std::tr1::shared_ptr<L_>, bp::bases<URILabel>, boost::noncopyable>
        (
         name.c_str(),
         "A concrete URI label class.",
         bp::init<const std::string &>("__init__(string)")
        )
    {
        bp::implicitly_convertible<std::tr1::shared_ptr<L_>, std::tr1::shared_ptr<URILabel> >();
    }
};

template <typename L_>
struct class_concrete_dependencies_label :
    bp::class_<L_, std::tr1::shared_ptr<L_>, bp::bases<DependenciesLabel>, boost::noncopyable>
{
    class_concrete_dependencies_label(const std::string & name) :
        bp::class_<L_, std::tr1::shared_ptr<L_>, bp::bases<DependenciesLabel>, boost::noncopyable>
        (
         name.c_str(),
         "A concrete dependencies label class.",
         bp::init<const std::string &>("__init__(string)")
        )
    {
        bp::implicitly_convertible<std::tr1::shared_ptr<L_>, std::tr1::shared_ptr<DependenciesLabel> >();
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
     * DependenciesLabel
     */
    register_shared_ptrs_to_python<DependenciesLabel>();
    bp::class_<DependenciesLabel, boost::noncopyable>
        (
         "DependenciesLabel",
         "Dependencies label base class.",
         bp::no_init
        )
        .add_property("text", &DependenciesLabel::text,
                "[ro] string\n"
                "Our text."
                )

        .def("__str__", &DependenciesLabel::text)
        ;

    /**
     * ConcreteDependenciesLabels
     */
    class_concrete_dependencies_label<DependenciesBuildLabel>("DependenciesBuildLabel");
    class_concrete_dependencies_label<DependenciesRunLabel>("DependenciesRunLabel");
    class_concrete_dependencies_label<DependenciesPostLabel>("DependenciesPostLabel");
    class_concrete_dependencies_label<DependenciesCompileAgainstLabel>("DependenciesCompileAgainstLabel");
    class_concrete_dependencies_label<DependenciesInstallLabel>("DependenciesInstallLabel");
    class_concrete_dependencies_label<DependenciesFetchLabel>("DependenciesFetchLabel");
    class_concrete_dependencies_label<DependenciesSuggestionLabel>("DependenciesSuggestionLabel");
    class_concrete_dependencies_label<DependenciesRecommendationLabel>("DependenciesRecommendationLabel");
}
