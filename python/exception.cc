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

#include "exception.hh"
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/instantiation_policy-impl.hh>
#include <map>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

template class InstantiationPolicy<ExceptionRegister, instantiation_method::SingletonTag>;

RegisteredExceptionBase::~RegisteredExceptionBase()
{
}

namespace paludis
{
    template<>
    struct Implementation<ExceptionRegister>
    {
        std::map<std::string, tr1::shared_ptr<RegisteredExceptionBase> > exceptions;
    };
    typedef std::map<std::string, tr1::shared_ptr<RegisteredExceptionBase> >::iterator ExceptionsIterator;
}

ExceptionRegister::ExceptionRegister() :
    PrivateImplementationPattern<ExceptionRegister>(new Implementation<ExceptionRegister>)
{
}

ExceptionRegister::~ExceptionRegister()
{
}

void
ExceptionRegister::add_map_item(const std::string & name, tr1::shared_ptr<RegisteredExceptionBase> p)
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
        return NULL;
}

void PALUDIS_VISIBLE expose_exception()
{
    ExceptionRegister::get_instance()->add_exception<Exception>("BaseException");
    ExceptionRegister::get_instance()->add_exception<InternalError>("InternalError", "BaseException");
    ExceptionRegister::get_instance()->add_exception<NotAvailableError>("NotAvailableError", "BaseException");
    ExceptionRegister::get_instance()->add_exception<NameError>("NameError", "BaseException");
    ExceptionRegister::get_instance()->add_exception<ConfigurationError>("ConfigurationError", "BaseException");
}
