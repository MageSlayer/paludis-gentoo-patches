/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_EAPI_FWD_HH
#define PALUDIS_GUARD_PALUDIS_EAPI_FWD_HH 1

#include <paludis/repositories/e/dep_parser-fwd.hh>
#include <paludis/util/fs_path-fwd.hh>
#include <paludis/fs_merger-fwd.hh>
#include <paludis/name-fwd.hh>
#include <paludis/elike_package_dep_spec-fwd.hh>
#include <memory>

namespace paludis
{
    namespace erepository
    {
        struct EAPIAnnotations;
        class EAPIData;
        class EAPIConfigurationError;
        struct EAPIEbuildPhases;
        struct EAPIEbuildMetadataVariables;
        struct EAPIEbuildOptions;
        class EAPILabels;
        struct EAPIToolsOptions;
        struct EAPIPipeCommands;
        struct EAPIProfileOptions;
        struct EAPIChoicesOptions;
        struct EAPIEbuildEnvironmentVariables;
        struct SupportedEAPI;
        struct EAPI;

        struct EAPIMetadataVariable;

        typedef std::function<const std::string (const FSPath &)> EAPIForFileFunction;

    }
}

#endif
