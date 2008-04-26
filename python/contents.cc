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
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/contents.hh>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

// For classes derived from ContentsEntry
template <typename C_>
class class_contents:
    public bp::class_<C_, std::tr1::shared_ptr<C_>, bp::bases<ContentsEntry>, boost::noncopyable>
{
    public:
        template <class Init_>
        class_contents(const std::string & name, const std::string & class_doc, Init_ initspec) :
            bp::class_<C_, std::tr1::shared_ptr<C_>, bp::bases<ContentsEntry>, boost::noncopyable>(
                    name.c_str(), class_doc.c_str(), initspec)
        {
            bp::register_ptr_to_python<std::tr1::shared_ptr<const C_> >();
            bp::implicitly_convertible<std::tr1::shared_ptr<C_>, std::tr1::shared_ptr<ContentsEntry> >();
        }
};

void expose_contents()
{
    /**
     * ContentsEntry
     */
    bp::register_ptr_to_python<std::tr1::shared_ptr<const ContentsEntry> >();
    bp::implicitly_convertible<std::tr1::shared_ptr<ContentsEntry>,
            std::tr1::shared_ptr<const ContentsEntry> >();
    bp::class_<ContentsEntry, boost::noncopyable>
        (
         "ContentsEntry",
         "Base class for a contents entry.",
         bp::no_init
        )
        .add_property("name", &ContentsEntry::name,
                "[ro] string\n"
                "Our name."
                )

        .def(bp::self_ns::str(bp::self))
        ;

    /**
     * ContentsFileEntry
     */
    class_contents<ContentsFileEntry>
        (
         "ContentsFileEntry",
         "A file contents entry.",
         bp::init<const std::string &>("__init__(name_string)")
        );

    /**
     * ContentsDirEntry
     */
    class_contents<ContentsDirEntry>
        (
         "ContentsDirEntry",
         "A directory contents entry.",
         bp::init<const std::string &>("__init__(name_string)")
        );

    /**
     * ContentsMiscEntry
     */
    class_contents<ContentsMiscEntry>
        (
         "ContentsMiscEntry",
         "A misc contents entry.",
         bp::init<const std::string &>("__init__(name_string)")
        );

    /**
     * ContentsFifoEntry
     */
    class_contents<ContentsFifoEntry>
        (
         "ContentsFifoEntry",
         "A fifo contents entry.",
         bp::init<const std::string &>("__init__(name_string)")
        );

    /**
     * ContentsDevEntry
     */
    class_contents<ContentsDevEntry>
        (
         "ContentsDevEntry",
         "A dev contents entry.",
         bp::init<const std::string &>("__init__(name_string)")
        );

    /**
     * ContentsSymEntry
     */
    class_contents<ContentsSymEntry>
        (
         "ContentsSymEntry",
         "A sym contents entry.",
         bp::init<const std::string &, const std::string &>("__init__(name_string, target_string)")
        )
        .add_property("target", &ContentsSymEntry::target,
                "[ro] string\n"
                "Our target (as per readlink)."
                )

        .def(bp::self_ns::str(bp::self))
        ;

    /**
     * Contents
     */
    register_shared_ptrs_to_python<Contents>(rsp_const);
    bp::class_<Contents, std::tr1::shared_ptr<Contents>, boost::noncopyable>
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
