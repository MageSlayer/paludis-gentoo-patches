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

#include <paludis/dep_tag.hh>
#include <paludis/dep_spec.hh>
#include <paludis/util/wrapped_forward_iterator.hh>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

struct DepTagCategoryFactoryWrapper
{
    // More convenient way of creating DepTagCategories
    static std::shared_ptr<const DepTagCategory>
    create(const DepTagCategoryFactory & self, const std::string & id)
    {
        return self.create(id);
    }
};

void expose_dep_tag()
{
    /**
     * DepTagCategory
     */
    register_shared_ptrs_to_python<DepTagCategory>();
    bp::class_<DepTagCategory, boost::noncopyable>
        (
         "DepTagCategory",
         "A DepTagCategory is identified by its name and has associated display "
         "information for a DepTag's category.",
         bp::no_init
        )
        .add_property("visible", &DepTagCategory::visible,
                "[ro] bool\n"
                "Should we be displayed in a tag category summary?"
                )

        .add_property("id", &DepTagCategory::id,
                "[ro] string\n"
                "Fetch our short ID (for example, 'GLSA')."
                )

        .add_property("title", &DepTagCategory::title,
                "[ro] string\n"
                "Fetch our title (for example, 'Security advisories'), or an "
                "empty string if we're untitled."
                )

        .add_property("pre_text", &DepTagCategory::pre_text,
                "[ro] string\n"
                "Fetch our pre list text, or an empty string."
                )

        .add_property("post_text", &DepTagCategory::post_text,
                "[ro] string\n"
                "Fetch our post list text, or an empty string."
                )
        ;

    /**
     * DepTagCategoryFactory
     */
    bp::class_<DepTagCategoryFactory, boost::noncopyable>
        (
         "DepTagCategoryFactory",
         "Virtual constructor for accessing DepTagCategory instances.",
         bp::no_init
        )
        .add_static_property("instance", bp::make_function(&DepTagCategoryFactory::get_instance,
                    bp::return_value_policy<bp::reference_existing_object>()),
                "Singleton instance."
                )

        .def("create", &DepTagCategoryFactory::create,
                "create(id_string) -> DepTagCategory\n"
                "Make DepTagCategory from id."
            )
        ;

    /**
     * DepTag
     */
    bp::class_<DepTag, boost::noncopyable>
        (
         "DepTag",
         "A DepTag can be associated with a PackageDepSpec, and is transferred "
         "onto any associated DepListEntry instances.",
         bp::no_init
        )
        .add_property("short_text", &DepTag::short_text,
                "[ro] string\n"
                "Our short text (for example, 'GLSA-1234') that is "
                "displayed with the dep list entry."
                )

        .add_property("category", &DepTag::category,
                "[ro] string\n"
                "Our DepTagCategory's tag."
                )

        .def("__cmp__", &py_cmp<DepTag>)
        ;

    /**
     * GLSADepTag
     */
    bp::class_<GLSADepTag, bp::bases<DepTag>, boost::noncopyable>
        (
         "GLSADepTag",
         "DepTag subclass for GLSAs.",
         bp::init<const std::string &, const std::string &, const FSEntry &>("__init__(id_str, glsa_title_str, glsa_file_str)")
        )
        .add_property("glsa_title", &GLSADepTag::glsa_title,
                "Our GLSA title (for example, 'Yet another PHP remote access hole')"
                )
        .add_property("glsa_file", &GLSADepTag::glsa_file,
                "Our GLSA filename"
                )
        ;

    /**
     * GeneralSetDepTag
     */
    bp::class_<GeneralSetDepTag, bp::bases<DepTag>, boost::noncopyable>
        (
         "GeneralSetDepTag",
         "DepTag subclass for general sets.",
         bp::init<const SetName &, const std::string &>("__init__(SetName, source_str)")
        )
        .add_property("source", &GeneralSetDepTag::source,
                "From which repository or environment did we originate?"
                )
        ;

    /**
     * DependencyDepTag
     */
    bp::class_<DependencyDepTag, bp::bases<DepTag>, boost::noncopyable>
        (
         "DependencyDepTag",
         "DepTag subclass for dependencies.",
         bp::init<const std::shared_ptr<const PackageID> &, const PackageDepSpec &>(
                    "__init__(PackageID, PackageDepSpec)"
                    )
        )
        .add_property("package_id", &DependencyDepTag::package_id,
                "[ro] PackageID\n"
                "The PackageID that contains our dependency."
                )

        .add_property("dependency", bp::make_function(&DependencyDepTag::dependency,
                    bp::return_value_policy<bp::return_by_value>()),
                "[ro] PackageDepSpec\n"
                "The PackageDepSpec that pulled us in."
                )
        ;

    /**
     * TargetDepTag
     */
    bp::class_<TargetDepTag, bp::bases<DepTag>, boost::noncopyable>
        (
         "TargetDepTag",
         "DepTag subclass for explicit targets.",
         bp::init<>("__init__()")
        )
        ;

    /**
     * DepListEntryTags
     */
    class_iterable<DepListEntryTags>
        (
         "DepListEntryTags",
         "Tags attached to a DepListEntry."
        );
}
