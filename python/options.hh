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
        template <typename O_>
        struct RegisterOptionsFromPython;

        template <typename E_>
        struct RegisterOptionsFromPython<Options<E_> >
        {
            static std::string _name;

            RegisterOptionsFromPython(const std::string & name)
            {
                boost::python::converter::registry::push_back(&convertible, &construct,
                        boost::python::type_id<Options<E_> >());

                _name = name;
            }

            static void *
            convertible(PyObject * obj_ptr)
            {
                if (boost::python::extract<boost::python::list>(obj_ptr).check())
                    return obj_ptr;
                else
                    return 0;
            }

            static void
            construct(PyObject * obj_ptr, boost::python::converter::rvalue_from_python_stage1_data * data)
            {
                typedef boost::python::converter::rvalue_from_python_storage<Options<E_> > Storage;
                void * storage = reinterpret_cast<Storage *>(data)->storage.bytes;

                data->convertible = storage;

                new (storage) Options<E_>;

                Options<E_> * o(reinterpret_cast<Options<E_> *>(storage));

                boost::python::list l = boost::python::extract<boost::python::list>(obj_ptr);

                while (PyList_Size(obj_ptr))
                {
                    boost::python::object py_e(l.pop());
                    if (boost::python::extract<E_>(py_e).check())
                    {
                        E_ e = boost::python::extract<E_>(py_e);
                        *o += e;
                    }
                    else
                    {
                        throw PythonError(std::string("Cannot add object of type ")
                                + py_e.ptr()->ob_type->tp_name + " to " + _name);
                    }
                }
            }
        };
        template <typename E_>
        std::string RegisterOptionsFromPython<Options<E_> >::_name("unknown");

        // expose Options classes
        template <typename O_>
        class class_options :
            public boost::python::class_<O_>
        {
            public:
                class_options(const std::string & set_name, const std::string & bit_name,
                        const std::string & class_doc) :
                    boost::python::class_<O_>(set_name.c_str(),
                            (class_doc + "\n\n Note that python lists containing suitable enum "
                             "values are convertible to Options classes.").c_str(),
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

                    RegisterOptionsFromPython<O_> tmp(set_name);
                }
        };
    } // namespace paludis::python
} // namespace paludis

#endif
