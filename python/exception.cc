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

#include "exception.hh"
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/singleton-impl.hh>
#include <python/paludis_python.hh>
#include <map>

#include <iostream>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

template class Singleton<ExceptionRegister>;

RegisteredExceptionBase::~RegisteredExceptionBase()
{
}

namespace paludis
{
    template<>
    struct Imp<ExceptionRegister>
    {
        std::map<std::string, std::shared_ptr<RegisteredExceptionBase> > exceptions;
    };
    typedef std::map<std::string, std::shared_ptr<RegisteredExceptionBase> >::iterator ExceptionsIterator;
}

ExceptionRegister::ExceptionRegister() :
    Pimp<ExceptionRegister>()
{
}

ExceptionRegister::~ExceptionRegister()
{
}

void
ExceptionRegister::add_map_item(const std::string & name, std::shared_ptr<RegisteredExceptionBase> p)
{
    _imp->exceptions.insert(std::make_pair(name, p));
}

PyObject *
ExceptionRegister::get_py_exception(const std::string & name)
{
    ExceptionsIterator i(_imp->exceptions.find(name));
    if (i != _imp->exceptions.end())
        return i->second->get_py_exception();
    else
    {
        std::cerr << "Exception '" << name << "' not found." << std::endl;
        throw PythonError("Exception '" + name + "' not found.");
    }
}

PythonError::PythonError(const std::string & message) throw () :
    Exception(message)
{
}

PythonMethodNotImplemented::PythonMethodNotImplemented(const std::string & class_name,
        const std::string & method_name) throw () :
    PythonError("Python subclasses of '" + class_name + "' have to implement '" + method_name + "' method")
{
}

PythonContainerConversionError::PythonContainerConversionError(const std::string & class_name,
        const std::string & container_name, const std::string & o_type) throw () :
    PythonError("Cannot add object of type '" + o_type + "' to a '"
            + class_name + "' " + container_name + " container.")
{
}

void expose_exception()
{
    /**
     * Exceptions
     */
    ExceptionRegister::get_instance()->add_exception<Exception>("BaseException",
            "Base exception class.");
    ExceptionRegister::get_instance()->add_exception<InternalError>("InternalError", "BaseException",
            "An InternalError is an Exception that is thrown if something "
            "that is never supposed to happen happens.");
    ExceptionRegister::get_instance()->add_exception<NotAvailableError>("NotAvailableError", "BaseException",
            "A NotAvailableError is an Exception that is thrown if something "
            "that is not available (for example due to compile time configure options "
            "or platform limitations) is used.");
    ExceptionRegister::get_instance()->add_exception<NameError>("NameError", "BaseException",
            "A NameError is an Exception that is thrown when some kind of invalid name is encountered.");
    ExceptionRegister::get_instance()->add_exception<ConfigurationError>("ConfigurationError", "BaseException",
            "A ConfigurationError is thrown when an invalid configuration occurs.");
    ExceptionRegister::get_instance()->add_exception<PythonError>("PythonError", "BaseException",
            "Base exception class for Python specific stuff.");
    ExceptionRegister::get_instance()->add_exception<PythonMethodNotImplemented>("PythonMethodNotImplemented",
            "PythonError",
            "Thrown if a not implemented virtual function was called from C++.");
    ExceptionRegister::get_instance()->add_exception<PythonContainerConversionError>(
            "PythonContainerConversionError",
            "PythonError",
            "Thrown if an error occurs during container conversion from python.");

}
