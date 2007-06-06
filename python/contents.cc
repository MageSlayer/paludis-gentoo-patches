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
#include <paludis/contents.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

// For classes derived from ContentsEntry
template <typename C_>
class class_contents:
    public bp::class_<C_, tr1::shared_ptr<C_>, bp::bases<ContentsEntry>, boost::noncopyable>
{
    public:
        template <class Init_>
        class_contents(const std::string & name, const std::string & class_doc, Init_ initspec) :
            bp::class_<C_, tr1::shared_ptr<C_>, bp::bases<ContentsEntry>, boost::noncopyable>(
                    name.c_str(), class_doc.c_str(), initspec)
        {
            bp::register_ptr_to_python<tr1::shared_ptr<const C_> >();
            bp::implicitly_convertible<tr1::shared_ptr<C_>, tr1::shared_ptr<ContentsEntry> >();
        }
};

void PALUDIS_VISIBLE expose_contents()
{
    bp::register_ptr_to_python<tr1::shared_ptr<const ContentsEntry> >();
    bp::implicitly_convertible<tr1::shared_ptr<ContentsEntry>,
            tr1::shared_ptr<const ContentsEntry> >();
    bp::class_<ContentsEntry, boost::noncopyable>
        ce("ContentsEntry",
                "Base class for a contents entry.",
                bp::no_init
          );
    ce.add_property("name", &ContentsEntry::name,
            "[ro] string\n"
            "Our name."
            );
    ce.def(bp::self_ns::str(bp::self));

    class_contents<ContentsFileEntry>
        cfilee("ContentsFileEntry",
                "A file contents entry.",
                bp::init<const std::string &>("__init__(name_string)")
           );

    class_contents<ContentsDirEntry>
        cdire("ContentsDirEntry",
                "A directory contents entry.",
                bp::init<const std::string &>("__init__(name_string)")
           );

    class_contents<ContentsMiscEntry>
        cme("ContentsMiscEntry",
                "A misc contents entry.",
                bp::init<const std::string &>("__init__(name_string)")
           );

    class_contents<ContentsFifoEntry>
        cfifoe("ContentsFifoEntry",
                "A fifo contents entry.",
                bp::init<const std::string &>("__init__(name_string)")
           );

    class_contents<ContentsDevEntry>
        cdeve("ContentsDevEntry",
                "A dev contents entry.",
                bp::init<const std::string &>("__init__(name_string)")
           );

    class_contents<ContentsSymEntry>
        cse("ContentsSymEntry",
                "A sym contents entry.",
                bp::init<const std::string &, const std::string &>("__init__(name_string, target_string)")
           );
    cse.add_property("target", &ContentsSymEntry::target,
            "[ro] string\n"
            "Our target (as per readlink)."
            );
    cse.def(bp::self_ns::str(bp::self));

    register_shared_ptrs_to_python<Contents>();
    bp::class_<Contents, boost::noncopyable>
        c("Contents",
                "Iterable of ContentsEntry.\n"
                "A package's contents.",
                bp::init<>("__init__()")
           );
    c.def("add", &Contents::add,
            "add(ContentsEntry)\n"
            "Add a new entry."
         );
    c.def("__iter__", bp::range(&Contents::begin, &Contents::end));
}

