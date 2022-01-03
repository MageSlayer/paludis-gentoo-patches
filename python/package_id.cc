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
#include <python/iterable.hh>

#include <paludis/package_id.hh>
#include <paludis/repositories/fake/fake_package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <paludis/action.hh>
#include <paludis/dep_spec.hh>
#include <paludis/contents.hh>
#include <paludis/util/wrapped_forward_iterator.hh>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

struct PackageIDWrapper
{
    static PyObject *
    find_metadata(const PackageID & self, const std::string & key)
    {
        PackageID::MetadataConstIterator i(self.find_metadata(key));
        if (i != self.end_metadata())
            return bp::incref(bp::object(*i).ptr());
        else
            return Py_None;
    }
};

void expose_package_id()
{
    /**
     * Enums
     */
    enum_auto("PackageIDCanonicalForm", last_idcf,
            "How to generate paludis::PackageID::canonical_form().");

    /**
     * PackageID
     */
    register_shared_ptrs_to_python<PackageID>();
    bp::class_<PackageID, boost::noncopyable>
        (
         "PackageID",
         bp::no_init
        )
        .def("canonical_form", &PackageID::canonical_form,
                "canonical_form(PackageIDCanonicalForm) -> string\n"
                )

        .add_property("name", &PackageID::name,
                "[ro] QualifiedPackageName\n"
                )

        .add_property("version", &PackageID::version,
                "[ro] VersionSpec\n"
                )

        .add_property("repository_name", &PackageID::repository_name,
                "[ro] RepositoryName\n"
                )

        .def("supports_action", &PackageID::supports_action,
                "supports_action(SupportsActionTestBase) -> bool\n"
                "NEED_DOC"
            )

        .def("perform_action", &PackageID::perform_action,
                "perform_action(Action)\n"
                "NEED_DOC"
            )

        .add_property("metadata", bp::range(&PackageID::begin_metadata, &PackageID::end_metadata),
                "[ro] Iterable of MetadataKey\n"
                "NEED_DOC"
                )

        .def("find_metadata", &PackageIDWrapper::find_metadata,
                "find_metadata(string) -> MetadataKey\n"
                "NEED_DOC"
            )

        .add_property("masked", &PackageID::masked,
                "[ro] bool\n"
                "NEED_DOC"
                )

        .add_property("masks", bp::range(&PackageID::begin_masks, &PackageID::end_masks),
                "[ro] Iterable of Mask\n"
                "NEED_DOC"
                )

        .def("keywords_key", &PackageID::keywords_key,
                "The keywords_key, if not None, is used by FindUnusedPackagesTask\n"
                "to determine whether a package is unused."
            )

        .def("build_dependencies_target_key", &PackageID::build_dependencies_target_key,
                "The build_dependencies_target_key, if not None, indicates a package's\n"
                "build-time dependencies in target architecture."
            )

        .def("build_dependencies_host_key", &PackageID::build_dependencies_host_key,
                "The build_dependencies_host_key, if not None, indicates a package's\n"
                "build-time dependencies in host architecture."
            )

        .def("run_dependencies_target_key", &PackageID::run_dependencies_target_key,
                "The run_dependencies_target_key, if not None, indicates a package's\n"
                "run-time dependencies in target architecture."
            )

        .def("run_dependencies_host_key", &PackageID::run_dependencies_host_key,
                "The run_dependencies_key, if not None, indicates a package's\n"
                "run-time dependencies in host architecture."
            )

        .def("post_dependencies_key", &PackageID::post_dependencies_key,
                "The post_dependencies_key, if not None, indicates a package's\n"
                "post-merge dependencies."
            )

        .def("fetches_key", &PackageID::fetches_key,
                "The fetches_key, if not None, indicates files that have to be fetched\n"
                "in order to install a package."
            )

        .def("homepage_key", &PackageID::homepage_key,
                "The homepage_key, if not None, describes a package's homepages."
            )

        .def("short_description_key", &PackageID::short_description_key,
                "The short_description_key, if not None, provides a short (no more\n"
                "than a few hundred characters) description of a package."
            )

        .def("long_description_key", &PackageID::long_description_key,
                "The long_description_key, if not None, provides a long\n"
                "description of a package."
            )

        .def("installed_time_key", &PackageID::installed_time_key,
                "The installed_time_key, if not None, contains the time a package\n"
                "was installed. It affects dependency resolution if DepList is\n"
                "using dl_reinstall_scm_daily or dl_reinstall_scm_weekly."
            )

        .def("from_repositories_key", &PackageID::from_repositories_key,
                "The from_repositories_key, if not None, contains strings describing\n"
                "the repositories whence a package originated."
            )

        .def("fs_location_key", &PackageID::fs_location_key,
                "The fs_location_key, if not None, indicates the filesystem\n"
                "location (for example, the ebuild file or VDB directory) that\n"
                "best describes the location of a PackageID."
            )

        .def("slot_key", &PackageID::slot_key,
                "The slot_key, if not None, indicates the package's slot."
            )

        .def("choices_key", &PackageID::choices_key,
                "The choices_key, if not None, indicates the package's choices."
            )

        .def("uniquely_identifying_spec", &PackageID::uniquely_identifying_spec,
                "A PackageDepSpec that uniquely identifies us.\n\n"
                "When stringified, can be turned back into an equivalent unique "
                "PackageDepSpec by using parse_user_package_dep_spec."
                )

        .def("contents", &PackageID::contents,
                "The contents, if not None, contains the contents of a\n"
                "package. For installed packages, this means the files installed;\n"
                "for installable packages, this means the files that will be\n"
                "installed (if known, which it may be for some binary packages)."
            )

        .def("__eq__", &py_eq<PackageID>)

        .def("__ne__", &py_ne<PackageID>)

        .def(bp::self_ns::str(bp::self))
        ;

    /**
     * PackageIDIterable
     */
    class_iterable<PackageIDSequence>
        (
         "PackageIDIterable",
         "Iterable of PackageID",
         true
        );
}
