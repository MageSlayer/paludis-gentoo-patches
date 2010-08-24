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

#ifndef PALUDIS_GUARD_PYTHON_ITERABLE_HH
#define PALUDIS_GUARD_PYTHON_ITERABLE_HH 1

#include <python/paludis_python.hh>
#include <python/exception.hh>

#include <paludis/util/sequence.hh>
#include <paludis/util/set.hh>
#include <paludis/util/fs_path.hh>
#include <type_traits>

namespace paludis
{
    namespace python
    {
        template <typename From_, typename To_>
        struct IsConvertible
        {
            static const bool value = std::is_convertible<From_, To_>::value;
        };

        template <>
        struct IsConvertible<std::string, FSPath>
        {
            static const bool value = true;
        };

        template <typename From_, typename Tag_>
        struct IsConvertible<From_, WrappedValue<Tag_> >
        {
            static const bool value = std::is_convertible<From_, typename WrappedValueTraits<Tag_>::UnderlyingType>::value;
        };

        template <typename To_, typename From_, typename C_, bool>
        struct ConditionalAdd
        {
            static void add(C_ &, PyObject * ptr)
            {
            }
        };

        template <typename To_>
        struct ConditionalAdd<To_, std::string, Set<To_>, true>
        {
            static void add(Set<To_> & c, PyObject * ptr)
            {
                const char * str = PyString_AsString(ptr);
                c.insert(To_(std::string(str)));
            }
        };

        template <typename To_>
        struct ConditionalAdd<To_, std::string, Sequence<To_>, true>
        {
            static void add(Sequence<To_> & c, PyObject * ptr)
            {
                const char * str = PyString_AsString(ptr);
                c.push_back(To_(std::string(str)));
            }
        };

        template <typename V_>
        struct RegisterSequenceSPTRFromPython
        {
            static std::string _name;

            RegisterSequenceSPTRFromPython(const std::string & name)
            {
                boost::python::converter::registry::push_back(&convertible, &construct,
                        boost::python::type_id<std::shared_ptr<Sequence<V_> > >());

                _name = name;
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
                typedef boost::python::converter::rvalue_from_python_storage<std::shared_ptr<Sequence<V_> > >
                    Storage;
                void * storage = reinterpret_cast<Storage *>(data)->storage.bytes;

                data->convertible = storage;

                if (obj_ptr == Py_None)
                {
                    new (storage) std::shared_ptr<Sequence<V_> >();
                    return;
                }
                else
                    new (storage) std::shared_ptr<Sequence<V_> >(new Sequence<V_>());

                Sequence<V_> * s(reinterpret_cast<std::shared_ptr<Sequence<V_> > *>(storage)->get());

                boost::python::list l = boost::python::extract<boost::python::list>(obj_ptr);

                while (PyList_Size(obj_ptr))
                {
                    boost::python::object o(l.pop());
                    if (boost::python::extract<V_ *>(o).check())
                    {
                        V_ * ptr = boost::python::extract<V_ *>(o);
                        s->push_back(*ptr);
                    }
                    else if (IsConvertible<std::string, V_>::value && PyString_Check(o.ptr()))
                    {
                        ConditionalAdd<V_, std::string, Sequence<V_>,
                            IsConvertible<std::string, V_>::value>::add(*s, o.ptr());
                    }
                    else
                    {
                        typedef std::shared_ptr<Sequence<V_> > sptr;
                        reinterpret_cast<sptr *>(storage)->sptr::~shared_ptr();

                        throw PythonContainerConversionError(_name, "sequence", o.ptr()->ob_type->tp_name);
                    }
                }
            }
        };
        template <typename V_>
        std::string RegisterSequenceSPTRFromPython<V_>::_name("unknown");

        template <typename V_>
        struct RegisterSetSPTRFromPython
        {
            static std::string _name;

            RegisterSetSPTRFromPython(const std::string & name)
            {
                boost::python::converter::registry::push_back(&convertible, &construct,
                        boost::python::type_id<std::shared_ptr<Set<V_> > >());

                _name = name;
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
                typedef boost::python::converter::rvalue_from_python_storage<std::shared_ptr<Set<V_> > >
                    Storage;
                void * storage = reinterpret_cast<Storage *>(data)->storage.bytes;

                data->convertible = storage;

                if (obj_ptr == Py_None)
                {
                    new (storage) std::shared_ptr<Set<V_> >();
                    return;
                }
                else
                    new (storage) std::shared_ptr<Set<V_> >(new Set<V_>());

                Set<V_> * s(reinterpret_cast<std::shared_ptr<Set<V_> > *>(storage)->get());

                boost::python::list l = boost::python::extract<boost::python::list>(obj_ptr);

                while (PyList_Size(obj_ptr))
                {
                    boost::python::object o(l.pop());
                    if (boost::python::extract<V_ *>(o).check())
                    {
                        V_ * ptr = boost::python::extract<V_ *>(o);
                        s->insert(*ptr);
                    }
                    else if (IsConvertible<std::string, V_>::value && PyString_Check(o.ptr()))
                    {
                        ConditionalAdd<V_, std::string, Set<V_>,
                                IsConvertible<std::string, V_>::value>::add(*s, o.ptr());
                    }
                    else
                    {
                        typedef std::shared_ptr<Set<V_> > sptr;
                        reinterpret_cast<sptr *>(storage)->sptr::~shared_ptr();

                        throw PythonContainerConversionError(_name, "set", o.ptr()->ob_type->tp_name);
                    }
                }
            }
        };
        template <typename V_>
        std::string RegisterSetSPTRFromPython<V_>::_name("unknown");

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
                        if (std::is_same<C_, Sequence<typename C_::value_type> >::value)
                            RegisterSequenceSPTRFromPython<typename C_::value_type> tmp(name);
                        else if (std::is_same<C_, Set<typename C_::value_type> >::value)
                            RegisterSetSPTRFromPython<typename C_::value_type> tmp(name);
                        else
                            throw PythonError("Can't register l-value converter for '" + name +"'.");
                    }
                }
        };
    } // namespace paludis::python
} // namespace paludis

#endif
