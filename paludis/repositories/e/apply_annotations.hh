/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_APPLY_ANNOTATIONS_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_APPLY_ANNOTATIONS_HH 1

#include <paludis/repositories/e/eapi-fwd.hh>
#include <paludis/util/map-fwd.hh>
#include <paludis/dep_spec-fwd.hh>
#include <memory>

namespace paludis
{
    namespace erepository
    {
        void apply_annotations(
                const EAPI & eapi,
                const std::shared_ptr<DepSpec> & spec,
                const std::shared_ptr<BlockDepSpec> & if_block_spec,
                const std::shared_ptr<const Map<std::string, std::string> > & m);

        void apply_annotations_not_block(
                const EAPI & eapi,
                const std::shared_ptr<DepSpec> & spec,
                const std::shared_ptr<const Map<std::string, std::string> > & m);
    }
}

#endif
