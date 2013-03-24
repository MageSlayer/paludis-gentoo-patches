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
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/fs_path.hh>
#include <paludis/contents.hh>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

// For classes derived from ContentsEntry
template <typename C_>
class class_contents:
    public bp::class_<C_, std::shared_ptr<C_>, bp::bases<ContentsEntry>, boost::noncopyable>
{
    public:
        template <class Init_>
        class_contents(const std::string & name, const std::string & class_doc, Init_ initspec) :
            bp::class_<C_, std::shared_ptr<C_>, bp::bases<ContentsEntry>, boost::noncopyable>(
                    name.c_str(), class_doc.c_str(), initspec)
        {
            bp::register_ptr_to_python<std::shared_ptr<const C_> >();
            bp::implicitly_convertible<std::shared_ptr<C_>, std::shared_ptr<ContentsEntry> >();
        }
};

void expose_contents()
{
    /**
     * ContentsEntry
     */
    bp::register_ptr_to_python<std::shared_ptr<const ContentsEntry> >();
    bp::implicitly_convertible<std::shared_ptr<ContentsEntry>,
            std::shared_ptr<const ContentsEntry> >();
    bp::class_<ContentsEntry, boost::noncopyable>
        (
         "ContentsEntry",
         "Base class for a contents entry.",
         bp::no_init
        )

        .add_property("metadata", bp::range(&ContentsEntry::begin_metadata, &ContentsEntry::end_metadata),
                "[ro] Iterable of MetadataKey\n"
                "NEED_DOC"
                )

        .def("find_metadata", &ContentsEntry::find_metadata,
                "find_metadata(string) -> MetadataKey\n"
                "NEED_DOC"
            )

        .def("location_key", &ContentsEntry::location_key,
                "The location_key, which will not be None, provides the path on the filesystem\n"
                "of the entry. It is not modified for root."
            )
        ;

    /**
     * ContentsFileEntry
     */
    class_contents<ContentsFileEntry>
        (
         "ContentsFileEntry",
         "A file contents entry.",
         bp::init<const FSPath &, const std::string &>("__init__(location, part)")
        )
        .def("part_key", &ContentsFileEntry::part_key)
        ;

    /**
     * ContentsDirEntry
     */
    class_contents<ContentsDirEntry>
        (
         "ContentsDirEntry",
         "A directory contents entry.",
         bp::init<const FSPath &>("__init__(location)")
        );

    /**
     * ContentsOtherEntry
     */
    class_contents<ContentsOtherEntry>
        (
         "ContentsOtherEntry",
         "An 'other' contents entry.",
         bp::init<const FSPath &>("__init__(location)")
        );

    /**
     * ContentsSymEntry
     */
    class_contents<ContentsSymEntry>
        (
         "ContentsSymEntry",
         "A sym contents entry.",
         bp::init<const FSPath &, const std::string &, const std::string &>("__init__(location, target_string, part)")
        )

        .def("target_key", &ContentsSymEntry::target_key,
                "The target_key, which will not be None, holds the symlink's target (as per readlink)."
            )
        .def("part_key", &ContentsSymEntry::part_key)
        ;

    /**
     * Contents
     */
    register_shared_ptrs_to_python<Contents>(rsp_const);
    bp::class_<Contents, std::shared_ptr<Contents>, boost::noncopyable>
        (
         "Contents",
         "Iterable of ContentsEntry.\n"
         "A package's contents.",
         bp::init<>("__init__()")
        )
        .def("add", &Contents::add,
                "add(ContentsEntry)\n"
                "Add a new entry."
            )

        .def("__iter__", bp::range(&Contents::begin, &Contents::end))
        ;
}
