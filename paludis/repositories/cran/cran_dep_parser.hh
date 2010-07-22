/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Danny van Dyk
 * Copyright (c) 2007, 2008 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_CRAN_DEP_PARSER_HH
#define PALUDIS_GUARD_PALUDIS_CRAN_DEP_PARSER_HH 1

#include <paludis/dep_spec.hh>
#include <paludis/spec_tree-fwd.hh>
#include <string>

namespace paludis
{
    namespace cranrepository
    {
        std::shared_ptr<DependencySpecTree> parse_depends(const std::string & s) PALUDIS_VISIBLE;
    }
}

#endif
