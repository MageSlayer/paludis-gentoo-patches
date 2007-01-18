/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include "exceptions.hh"

using namespace paludis;

DepListError::DepListError(const std::string & m) throw () :
    Exception(m)
{
}

AllMaskedError::AllMaskedError(const std::string & q) throw () :
    DepListError("Error searching for '" + q + "': no available versions"),
    _query(q)
{
}

UseRequirementsNotMetError::UseRequirementsNotMetError(const std::string & q) throw () :
    DepListError("Error searching for '" + q + "': use requirements are not met"),
    _query(q)
{
}

BlockError::BlockError(const std::string & msg) throw () :
    DepListError("Block: " + msg)
{
}

CircularDependencyError::CircularDependencyError(const std::string & msg) throw () :
    DepListError("Circular dependency: " + msg)
{
}

DowngradeNotAllowedError::DowngradeNotAllowedError(const std::string & to, const std::string & from) throw () :
    DepListError("Downgrade to '" + to + "' from '" + from + "' forbidden")
{
}

DowngradeNotAllowedError::~DowngradeNotAllowedError() throw ()
{
}

