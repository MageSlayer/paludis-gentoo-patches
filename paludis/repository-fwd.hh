/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORY_FWD_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORY_FWD_HH 1

#include <paludis/util/set-fwd.hh>
#include <paludis/util/options-fwd.hh>
#include <paludis/repository-fwd.hh>
#include <memory>
#include <iosfwd>

/** \file
 * Forward declarations for paludis/repository.hh .
 *
 * \ingroup g_repository
 */

namespace paludis
{
    class NoSuchSetError;
    class RecursivelyDefinedSetError;

    class Environment;
    class RepositoryNameCache;

    class Repository;
    class RepositoryEnvironmentVariableInterface;
    class RepositoryVirtualsInterface;
    class RepositoryMakeVirtualsInterface;
    class RepositoryDestinationInterface;
    class RepositoryManifestInterface;

    struct MergeParams;

#include <paludis/repository-se.hh>

    typedef Options<RepositoryContentMayExclude> RepositoryContentMayExcludes;
}

#endif
