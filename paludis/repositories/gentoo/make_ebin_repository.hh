/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_GENTOO_MAKE_EBIN_REPOSITORY_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_GENTOO_MAKE_EBIN_REPOSITORY_HH 1

#include <paludis/repositories/gentoo/portage_repository.hh>

namespace paludis
{
    /**
     * Create an ebin format repository.
     *
     * \see PortageRepository
     * \ingroup grpportagerepository
     */
    std::tr1::shared_ptr<PortageRepository>
    make_ebin_repository(
            Environment * const,
            std::tr1::shared_ptr<const AssociativeCollection<std::string, std::string> >) PALUDIS_VISIBLE;

    std::tr1::shared_ptr<Repository>
    make_ebin_repository_wrapped(
            Environment * const env,
            std::tr1::shared_ptr<const AssociativeCollection<std::string, std::string> > m);
}


#endif
