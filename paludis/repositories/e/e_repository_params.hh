/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_E_REPOSITORY_PARAMS_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_E_REPOSITORY_PARAMS_HH 1

#include <paludis/util/fs_entry.hh>
#include <paludis/util/sr.hh>

/** \file
 * Declaration for the ERepositoryParams class.
 *
 * \ingroup grperepository
 */

namespace paludis
{
    class Environment;
    class PackageDatabase;
    class ERepository;

    typedef Sequence<std::tr1::shared_ptr<const ERepository> > ERepositorySequence;

    namespace erepository
    {
#include <paludis/repositories/e/e_repository_params-se.hh>
    }

#include <paludis/repositories/e/e_repository_params-sr.hh>

}

#endif
