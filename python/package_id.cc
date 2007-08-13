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
#include <python/iterable.hh>

#include <paludis/package_id.hh>
#include <paludis/repositories/fake/fake_package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <paludis/action.hh>
#include <paludis/util/iterator.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

struct PackageIDWrapper
{
    static PyObject *
    find_metadata(const PackageID & self, const std::string & key)
    {
        PackageID::MetadataIterator i(self.find_metadata(key));
        if (i != self.end_metadata())
            return bp::incref(bp::object(**i).ptr());
        else
            return Py_None;
    }

    static IndirectIterator<PackageID::MetadataIterator>
    begin_metadata(const PackageID & self)
    {
        return indirect_iterator(self.begin_metadata());
    }

    static IndirectIterator<PackageID::MetadataIterator>
    end_metadata(const PackageID & self)
    {
        return indirect_iterator(self.end_metadata());
    }
};

void PALUDIS_VISIBLE expose_package_id()
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

        .add_property("slot", &PackageID::slot,
                "[ro] SlotName\n"
                )

        .add_property("repository", &PackageID::repository,
                "[ro] Repository\n"
                )

        .def("supports_action", &PackageID::supports_action,
                "supports_action(SupportsActionTestBase) -> bool\n"
                "NEED_DOC"
            )

        .def("perform_action", &PackageID::perform_action,
                "perform_action(Action)\n"
                "NEED_DOC"
            )

        .add_property("metadata", bp::range(&PackageIDWrapper::begin_metadata, &PackageIDWrapper::end_metadata),
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

        .def("__eq__", &py_eq<PackageID>)

        .def("__ne__", &py_ne<PackageID>)
        ;

    /**
     * PackageIDIterable
     */
    class_iterable<PackageIDSequence>
        (
         "PackageIDIterable",
         "Iterable of PackageID"
        );
}
