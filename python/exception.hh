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

#ifndef PALUDIS_GUARD_PYTHON_EXCEPTION_HH
#define PALUDIS_GUARD_PYTHON_EXCEPTION_HH 1

#include <paludis/util/pimp.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/singleton-impl.hh>
#include <boost/python.hpp>
#include <memory>
#include <functional>

namespace paludis
{
    namespace python
    {
        class RegisteredExceptionBase
        {
            public:
                virtual ~RegisteredExceptionBase() = 0;

                virtual PyObject * get_py_exception() const = 0;
        };

        template <typename Ex_>
        class RegisteredException :
            public RegisteredExceptionBase
        {
            private:
                const std::string _name;
                const std::string _longname;
                PyObject * _e;

            public:
                RegisteredException(const std::string & name, const std::string & doc, PyObject * base);

                void translator(const Ex_ & x) const;

                PyObject *
                get_py_exception() const
                {
                    return _e;
                }
        };

        template <class Ex_>
        RegisteredException<Ex_>::RegisteredException(const std::string & name,
                const std::string & doc, PyObject * base) :
            _name(name),
            _longname("paludis." + name),
            _e(PyErr_NewException(const_cast<char*>(_longname.c_str()), base, NULL))
        {
            PyModule_AddObject(boost::python::detail::current_scope, const_cast<char*>(_name.c_str()), _e);
            PyObject * doc_string =
#if PY_MAJOR_VERSION < 3
                PyString_FromString(doc.c_str());
#   else
                PyUnicode_FromString(doc.c_str());
#   endif
            PyObject_SetAttrString(_e, "__doc__", doc_string);
            boost::python::register_exception_translator<Ex_>(
                    std::bind(std::mem_fn(&RegisteredException<Ex_>::translator), this, std::placeholders::_1));
        }

        template <class Ex_>
        void
        RegisteredException<Ex_>::translator(const Ex_ & x) const
        {
#if PY_MAJOR_VERSION < 3
            PyObject * backtrace = PyString_FromString(x.backtrace("\n").c_str());
            PyObject * message = PyString_FromString(x.message().c_str());
            PyObject * what = PyString_FromString(x.what());
#else
            PyObject * backtrace = PyUnicode_FromString(x.backtrace("\n").c_str());
            PyObject * message = PyUnicode_FromString(x.message().c_str());
            PyObject * what = PyUnicode_FromString(x.what());
#endif
            PyObject_SetAttrString(_e, "backtrace", backtrace);
            PyObject_SetAttrString(_e, "message", message);
            PyObject_SetAttrString(_e, "what", what);

            PyErr_SetString(_e, x.message().c_str());
        }

        class PALUDIS_VISIBLE ExceptionRegister :
            public Singleton<ExceptionRegister>
        {
            friend class Singleton<ExceptionRegister>;

            private:
                Pimp<ExceptionRegister> _imp;
                ExceptionRegister();

                void add_map_item(const std::string & name, std::shared_ptr<RegisteredExceptionBase>);
                PyObject * get_py_exception(const std::string & name);

            public:
                ~ExceptionRegister();

                template <typename Ex_>
                void add_exception(const std::string & name, const std::string & doc)
                {
                    add_map_item(name, std::shared_ptr<RegisteredExceptionBase>(
                                new RegisteredException<Ex_>(name, doc, 0)));
                }

                template <typename Ex_>
                void add_exception(const std::string & name, const std::string & base, const std::string & doc)
                {
                    add_map_item(name, std::shared_ptr<RegisteredExceptionBase>(
                                new RegisteredException<Ex_>(name, doc, get_py_exception(base))));
                }
        };

        class PALUDIS_VISIBLE PythonError :
            public Exception
        {
            public:
                PythonError(const std::string & message) throw ();
        };

        class PALUDIS_VISIBLE PythonMethodNotImplemented :
            public PythonError
        {
            public:
                PythonMethodNotImplemented(const std::string & class_name,
                        const std::string & function_name) throw ();
        };

        class PALUDIS_VISIBLE PythonContainerConversionError :
            public PythonError
        {
            public:
                PythonContainerConversionError(const std::string & class_name,
                        const std::string & container_name, const std::string & o_type) throw ();
        };
    }
}

#endif
