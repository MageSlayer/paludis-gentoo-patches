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

#ifndef PALUDIS_GUARD_PYTHON_PALUDIS_EXCEPTION_HH
#define PALUDIS_GUARD_PYTHON_PALUDIS_EXCEPTION_HH 1

#include <paludis/util/tr1_memory.hh>
#include <paludis/util/tr1_functional.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/exception.hh>

#include <boost/python.hpp>

using namespace paludis;
namespace bp = boost::python;

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
                RegisteredException(const std::string & name, PyObject * base);

                void translator(const Ex_ & x) const;

                PyObject *
                get_py_exception() const
                {
                    return _e;
                }
        };

        template <class Ex_>
        RegisteredException<Ex_>::RegisteredException(const std::string & name, PyObject * base=NULL) :
            _name(name),
            _longname("paludis." + name),
            _e(PyErr_NewException(const_cast<char*>(_longname.c_str()), base, NULL))
        {
            PyModule_AddObject(bp::detail::current_scope, const_cast<char*>(_name.c_str()), _e);
            bp::register_exception_translator<Ex_>(
                    tr1::bind(tr1::mem_fn(&RegisteredException<Ex_>::translator),
                        this, tr1::placeholders::_1)
                    );
        }

        template <class Ex_>
        void
        RegisteredException<Ex_>::translator(const Ex_ & x) const
        {
            PyObject * backtrace = PyString_FromString(x.backtrace("\n").c_str());
            PyObject * message = PyString_FromString(x.message().c_str());
            PyObject * what = PyString_FromString(x.what());
            PyObject_SetAttrString(_e, "backtrace", backtrace);
            PyObject_SetAttrString(_e, "message", message);
            PyObject_SetAttrString(_e, "what", what);

            PyErr_SetString(_e, x.message().c_str());
        }

        class PALUDIS_VISIBLE ExceptionRegister :
            public InstantiationPolicy<ExceptionRegister, instantiation_method::SingletonTag>,
            private PrivateImplementationPattern<ExceptionRegister>
        {
            friend class InstantiationPolicy<ExceptionRegister, instantiation_method::SingletonTag>;

            private:
                ExceptionRegister();

                void add_map_item(const std::string & name, tr1::shared_ptr<RegisteredExceptionBase>);
                PyObject * get_py_exception(const std::string & name);

            public:
                ~ExceptionRegister();

                template <typename Ex_>
                void add_exception(const std::string & name)
                {
                    add_map_item(name, tr1::shared_ptr<RegisteredExceptionBase>(
                                new RegisteredException<Ex_>(name)));
                }

                template <typename Ex_>
                void add_exception(const std::string & name, const std::string & base)
                {
                    add_map_item(name, tr1::shared_ptr<RegisteredExceptionBase>(
                                new RegisteredException<Ex_>(name, get_py_exception(base))));
                }
        };
    }
}

#endif

