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

#ifndef PALUDIS_GUARD_PYTHON_ITERABLE_HH
#define PALUDIS_GUARD_PYTHON_ITERABLE_HH 1

#include <python/paludis_python.hh>
#include <python/exception.hh>

#include <paludis/util/tr1_type_traits.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/set.hh>

#include <iostream>

namespace paludis
{
    namespace python
    {
        template <typename V_>
        struct RegisterSequenceSPTRFromPython
        {
            static std::string class_name;

            RegisterSequenceSPTRFromPython(const std::string & name)
            {
                boost::python::converter::registry::push_back(&convertible, &construct,
                        boost::python::type_id<tr1::shared_ptr<Sequence<V_> > >());

                class_name = name;
            }

            static void *
            convertible(PyObject * obj_ptr)
            {
                if (boost::python::extract<boost::python::list>(obj_ptr).check()
                        || obj_ptr == Py_None)
                    return obj_ptr;
                else
                    return 0;
            }

            static void
            construct(PyObject * obj_ptr, boost::python::converter::rvalue_from_python_stage1_data * data)
            {
                typedef boost::python::converter::rvalue_from_python_storage<tr1::shared_ptr<Sequence<V_> > >
                    Storage;
                void * storage = reinterpret_cast<Storage *>(data)->storage.bytes;

                data->convertible = storage;

                if (obj_ptr == Py_None)
                {
                    new (storage) tr1::shared_ptr<Sequence<V_> >();
                    return;
                }
                else
                    new (storage) tr1::shared_ptr<Sequence<V_> >(new Sequence<V_>());

                Sequence<V_> * s(reinterpret_cast<tr1::shared_ptr<Sequence<V_> > *>(storage)->get());

                boost::python::list l = boost::python::extract<boost::python::list>(obj_ptr);

                while (PyList_Size(obj_ptr))
                {
                    boost::python::object o(l.pop());
                    if (boost::python::extract<V_ *>(o).check())
                    {
                        V_ * ptr = boost::python::extract<V_ *>(o);
                        s->push_back(*ptr);
                    }
                    else
                    {
                        typedef tr1::shared_ptr<Sequence<V_> > sptr;
                        reinterpret_cast<sptr *>(storage)->sptr::~shared_ptr();

                        throw PythonContainerConversionError(class_name, "sequence", o.ptr()->ob_type->tp_name);
                    }
                }
            }
        };
        template <typename V_>
        std::string RegisterSequenceSPTRFromPython<V_>::class_name("unknown");

        template <typename V_>
        struct RegisterSetSPTRFromPython
        {
            static std::string class_name;

            RegisterSetSPTRFromPython(const std::string & name)
            {
                boost::python::converter::registry::push_back(&convertible, &construct,
                        boost::python::type_id<tr1::shared_ptr<Set<V_> > >());

                class_name = name;
            }

            static void *
            convertible(PyObject * obj_ptr)
            {
                if (boost::python::extract<boost::python::list>(obj_ptr).check()
                        || obj_ptr == Py_None)
                    return obj_ptr;
                else
                    return 0;
            }

            static void
            construct(PyObject * obj_ptr, boost::python::converter::rvalue_from_python_stage1_data * data)
            {
                typedef boost::python::converter::rvalue_from_python_storage<tr1::shared_ptr<Set<V_> > >
                    Storage;
                void * storage = reinterpret_cast<Storage *>(data)->storage.bytes;

                data->convertible = storage;

                if (obj_ptr == Py_None)
                {
                    new (storage) tr1::shared_ptr<Set<V_> >();
                    return;
                }
                else
                    new (storage) tr1::shared_ptr<Set<V_> >(new Set<V_>());

                Set<V_> * s(reinterpret_cast<tr1::shared_ptr<Set<V_> > *>(storage)->get());

                boost::python::list l = boost::python::extract<boost::python::list>(obj_ptr);

                while (PyList_Size(obj_ptr))
                {
                    boost::python::object o(l.pop());
                    if (boost::python::extract<V_ *>(o).check())
                    {
                        V_ * ptr = boost::python::extract<V_ *>(o);
                        s->insert(*ptr);
                    }
                    else
                    {
                        typedef tr1::shared_ptr<Set<V_> > sptr;
                        reinterpret_cast<sptr *>(storage)->sptr::~shared_ptr();

                        throw PythonContainerConversionError(class_name, "set", o.ptr()->ob_type->tp_name);
                    }
                }
            }
        };
        template <typename V_>
        std::string RegisterSetSPTRFromPython<V_>::class_name("unknown");

        // expose iterable classes
        template <typename C_>
        class class_iterable :
            public boost::python::class_<C_, boost::noncopyable>
        {
            public:
                class_iterable(const std::string & name, const std::string & class_doc, bool converter=false) :
                    boost::python::class_<C_, boost::noncopyable>(name.c_str(), class_doc.c_str(),
                            boost::python::no_init)
                {
                    this->def("__iter__", boost::python::range(&C_::begin, &C_::end));
                    register_shared_ptrs_to_python<C_>();

                    if (converter)
                    {
                        if (tr1::is_same<C_, Sequence<typename C_::value_type> >::value)
                            RegisterSequenceSPTRFromPython<typename C_::value_type> tmp(name);
                        else if (tr1::is_same<C_, Set<typename C_::value_type> >::value)
                            RegisterSetSPTRFromPython<typename C_::value_type> tmp(name);
                        else
                            throw PythonError("Can't register l-value converter for '" + name +"'.");
                    }
                }
        };
    } // namespace paludis::python
} // namespace paludis

#endif
