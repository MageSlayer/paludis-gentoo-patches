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

#ifndef PALUDIS_GUARD_PYTHON_PALUDIS_PYTHON_HH
#define PALUDIS_GUARD_PYTHON_PALUDIS_PYTHON_HH 1

#include <paludis/util/tr1_memory.hh>
#include <paludis/util/tr1_functional.hh>

#include <paludis/util/stringify.hh>

#include <boost/python.hpp>

using namespace paludis;
namespace bp = boost::python;

namespace paludis
{
    // Make Boost.Python work with tr1::shared_ptr<>
    template <typename T_>
    inline T_ * get_pointer(tr1::shared_ptr<T_> const & p)
    {
        return p.get();
    }

    // Make Boost.Python work with tr1::shared_ptr<const>
    template <typename T_>
    inline T_ * get_pointer(tr1::shared_ptr<const T_> const & p)
    {
        return const_cast<T_*>(p.get());
    }
}

namespace boost
{
    namespace python
    {
        // Make Boost.Python work with tr1::shared_ptr<>
        template <typename T_>
        struct pointee<tr1::shared_ptr<T_> >
        {
            typedef T_ type;
        };

        // Make Boost.Python work with tr1::shared_ptr<const>
        template <typename T_>
        struct pointee<tr1::shared_ptr<const T_> >
        {
            typedef T_ type;
        };
    }
}

namespace paludis
{
    namespace python
    {
        // register shared_ptrs
        template <typename T_>
        void
        register_shared_ptrs_to_python()
        {
            bp::register_ptr_to_python<tr1::shared_ptr<T_> >();
            bp::register_ptr_to_python<tr1::shared_ptr<const T_> >();
            bp::implicitly_convertible<tr1::shared_ptr<T_>, tr1::shared_ptr<const T_> >();
        }

        // expose stringifyable enums
        template <typename E_>
        void
        enum_auto(const std::string & name, E_ e_last)
        {
            bp::enum_<E_> enum_(name.c_str());
            for (E_ e(static_cast<E_>(0)); e != e_last ; e = static_cast<E_>(static_cast<int>(e) + 1))
            {
                const std::string e_name_low = stringify(e);
                std::string e_name_up;
                std::transform(e_name_low.begin(), e_name_low.end(), std::back_inserter(e_name_up), &::toupper);
                enum_.value(e_name_up.c_str(), e);
            }
        }

        // Compare
        template <typename T_>
        int __cmp__(T_ & a, T_ & b)
        {
            if (a == b)
                return 0;
            else if (a < b)
                return -1;
            else
                return 1;
        }

        // Not equal
        template <typename T_>
        bool __ne__(T_ & a, T_ & b)
        {
            return ! (a == b);
        }

        // Translates Paludis C++ exception to a Python one with output of message() and backtrace() saved
        // in the corresponding string attributes of the Python exception.
        template <typename Ex_>
        class register_exception
        {
            private:
                PyObject * _e;
                const std::string _name;
                const std::string _longname;

            public:
                register_exception(const std::string & name) :
                    _name(name),
                    _longname("paludis."+name)
                {
                    _e = PyErr_NewException(const_cast<char*>(_longname.c_str()), NULL, NULL);
                    PyModule_AddObject(bp::detail::current_scope, const_cast<char*>(_name.c_str()), _e);
                    bp::register_exception_translator<Ex_>(
                            tr1::bind(tr1::mem_fn(&register_exception<Ex_>::translator),
                                this, tr1::placeholders::_1)
                            );
                }

                void
                translator(const Ex_ & x) const
                {
                    PyObject * backtrace = PyString_FromString(x.backtrace("\n").c_str());
                    PyObject * message = PyString_FromString(x.message().c_str());
                    PyObject * what = PyString_FromString(x.what());
                    PyObject_SetAttrString(_e, "backtrace", backtrace);
                    PyObject_SetAttrString(_e, "message", message);
                    PyObject_SetAttrString(_e, "what", what);

                    PyErr_SetString(_e, x.message().c_str());
                }
        };

        // expose Validated classes
        template <typename V_, typename Data_=std::string>
        class class_validated :
            public bp::class_<V_>
        {
            public:
                class_validated(const std::string & name,
                        const std::string & class_doc, const std::string & init_arg="string") :
                    bp::class_<V_>(name.c_str(), class_doc.c_str(),
                            bp::init<const Data_ &>(("__init__("+init_arg+")").c_str())
                            )
                {
                    this->def(bp::self_ns::str(bp::self));
                    bp::implicitly_convertible<Data_, V_>();
                }
        };

        // expose *Collection classes
        template <typename C_>
        class class_collection :
            public bp::class_<C_, boost::noncopyable>
        {
            public:
                class_collection(const std::string & name, const std::string & class_doc) :
                    bp::class_<C_, boost::noncopyable>(name.c_str(), class_doc.c_str(), bp::no_init)
                {
                    this->def("__iter__", bp::range(&C_::begin, &C_::end));
                    register_shared_ptrs_to_python<C_>();
                }
        };

        // expose Options classes
        template <typename O_>
        class class_options :
            public bp::class_<O_>
        {
            public:
                class_options(const std::string & set_name, const std::string & bit_name,
                        const std::string & class_doc) :
                    bp::class_<O_>(set_name.c_str(), class_doc.c_str(), bp::init<>("__init__()"))
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
                    this->def("__iadd__", &O_::operator+=, bp::return_self<>(),
                            ("__iadd__("+bit_name+") -> "+set_name+"\n"
                             "Enable the specified bit.").c_str()
                            );
                    this->def("__sub__", &O_::operator-,
                            ("__sub__("+bit_name+") -> "+set_name+"\n"
                             "Return a copy of ourself with the specified bit disabled.").c_str()
                            );
                    this->def("__isub__", &O_::operator-=, bp::return_self<>(),
                            ("__isub__("+bit_name+") -> "+set_name+"\n"
                             "Disable the specified bit.").c_str()
                            );
                    this->def("__or__", &O_::operator|,
                            ("__or__("+set_name+") -> "+set_name+"\n"
                             "Return a copy of ourself, bitwise 'or'ed with another Options set.").c_str()
                            );
                    this->def("__ior__", &O_::operator|=, bp::return_self<>(),
                            ("__ior__("+set_name+") -> "+set_name+"\n"
                             "Enable any bits that are enabled in the parameter.").c_str()
                            );
                    this->def("__getitem__", &O_::operator[],
                            ("__getitem__("+bit_name+") -> bool\n"
                             "Returns whether the specified bit is enabled.").c_str()
                            );
                    this->def("subtract", &O_::subtract,  bp::return_self<>(),
                            ("subtract("+set_name+") -> "+set_name+"\n"
                             "Disable any bits that are enabled in the parameter.").c_str()
                            );
                }
        };

        // Convert to string
        template <typename T_>
        struct to_string
        {
            static PyObject *
            convert(const T_ & x)
            {
                return PyString_FromString(stringify<T_>(x).c_str());
            }
        };

        // Convert pair to tuple
        template <typename first_, typename second_>
        struct pair_to_tuple
        {
            static PyObject *
            convert(const std::pair<first_, second_> & x)
            {
                return bp::incref(bp::make_tuple(x.first, x.second).ptr());
            }
        };
    } // namespace paludis::python
} // namespace paludis

#endif
