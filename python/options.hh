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

#ifndef PALUDIS_GUARD_PYTHON_OPTIONS_HH
#define PALUDIS_GUARD_PYTHON_OPTIONS_HH 1

#include <python/paludis_python.hh>

#include <paludis/util/options.hh>

namespace paludis
{
    namespace python
    {
        // expose Options classes
        template <typename O_>
        class class_options :
            public boost::python::class_<O_>
        {
            public:
                class_options(const std::string & set_name, const std::string & bit_name,
                        const std::string & class_doc) :
                    boost::python::class_<O_>(set_name.c_str(), class_doc.c_str(),
                            boost::python::init<>("__init__()"))
                {
                    this->add_property("any", &O_::any,
                            "[ro] bool\n"
                            "Is any bit enabled."
                            );
                    this->add_property("none", &O_::none,
                            "[ro] bool\n"
                            "Are all bits disabled."
                            );
                    this->def("__add__", &O_::operator+,
                            ("__add__("+bit_name+") -> "+set_name+"\n"
                             "Return a copy of ourself with the specified bit enabled.").c_str()
                            );
                    this->def("__iadd__", &O_::operator+=, boost::python::return_self<>(),
                            ("__iadd__("+bit_name+") -> "+set_name+"\n"
                             "Enable the specified bit.").c_str()
                            );
                    this->def("__sub__", &O_::operator-,
                            ("__sub__("+bit_name+") -> "+set_name+"\n"
                             "Return a copy of ourself with the specified bit disabled.").c_str()
                            );
                    this->def("__isub__", &O_::operator-=, boost::python::return_self<>(),
                            ("__isub__("+bit_name+") -> "+set_name+"\n"
                             "Disable the specified bit.").c_str()
                            );
                    this->def("__or__", &O_::operator|,
                            ("__or__("+set_name+") -> "+set_name+"\n"
                             "Return a copy of ourself, bitwise 'or'ed with another Options set.").c_str()
                            );
                    this->def("__ior__", &O_::operator|=, boost::python::return_self<>(),
                            ("__ior__("+set_name+") -> "+set_name+"\n"
                             "Enable any bits that are enabled in the parameter.").c_str()
                            );
                    this->def("__getitem__", &O_::operator[],
                            ("__getitem__("+bit_name+") -> bool\n"
                             "Returns whether the specified bit is enabled.").c_str()
                            );
                    this->def("subtract", &O_::subtract,  boost::python::return_self<>(),
                            ("subtract("+set_name+") -> "+set_name+"\n"
                             "Disable any bits that are enabled in the parameter.").c_str()
                            );
                }
        };
    } // namespace paludis::python
} // namespace paludis

#endif
