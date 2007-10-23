/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#ifndef PALUDIS_GUARD_PALUDIS_VERSION_REQUIREMENTS_HH
#define PALUDIS_GUARD_PALUDIS_VERSION_REQUIREMENTS_HH 1

#include <paludis/util/sr.hh>
#include <paludis/version_requirements-fwd.hh>
#include <paludis/version_operator.hh>
#include <paludis/version_spec.hh>

/** \file
 * Declarations for version requirements classes.
 *
 * \ingroup g_dep_spec
 *
 * \section Examples
 *
 * - \ref example_dep_spec.cc "example_dep_spec.cc" (for specifications)
 */

namespace paludis
{
#include <paludis/version_requirements-sr.hh>
}

#endif
