/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#include <paludis/util/exception.hh>

/** \file
 * Exception class implementations.
 *
 * \ingroup grpexceptions
 */

using namespace paludis;

Exception::Exception(const std::string & message) throw () :
    _message(message)
{
}

Exception::~Exception() throw ()
{
}

const std::string &
Exception::message() const throw ()
{
    return _message;
}

InternalError::InternalError(const std::string & where, const std::string & message) throw () :
    Exception("Eek! Internal error at " + where + ": " + message)
{
}

InternalError::InternalError(const std::string & where) throw () :
    Exception("Eek! Internal error at " + where)
{
}

NameError::NameError(const std::string & name, const std::string & role) throw () :
    Exception("Name '" + name + "' is not a valid " + role)
{
}

ConfigurationError::ConfigurationError(const std::string & msg) throw () :
    Exception(msg)
{
}

