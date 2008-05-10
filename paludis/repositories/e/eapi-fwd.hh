/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
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

#ifndef PALUDIS_GUARD_PALUDIS_EAPI_FWD_HH
#define PALUDIS_GUARD_PALUDIS_EAPI_FWD_HH 1

#include <paludis/util/kc-fwd.hh>
#include <paludis/util/keys.hh>
#include <paludis/repositories/e/dep_parser-fwd.hh>
#include <paludis/merger-fwd.hh>
#include <paludis/name-fwd.hh>
#include <paludis/elike_package_dep_spec-fwd.hh>
#include <tr1/memory>

namespace paludis
{
    namespace erepository
    {
        class EAPIData;
        class EAPIConfigurationError;
        class EAPIEbuildPhases;
        class EAPIEbuildMetadataVariables;
        class EAPIEbuildOptions;
        class EAPILabels;
        class EAPIToolsOptions;

        typedef kc::KeyedClass<
            kc::Field<k::rewrite_virtuals, bool>,
            kc::Field<k::no_slot_or_repo, bool>
                > EAPIPipeCommands;

        /**
         * Information about a supported EAPI's ebuild environment variables.
         *
         * \see EAPIData
         * \see EAPI
         * \ingroup grpeapi
         * \nosubgrouping
         */
        typedef kc::KeyedClass<
            kc::Field<k::env_use, std::string>,
            kc::Field<k::env_use_expand, std::string>,
            kc::Field<k::env_use_expand_hidden, std::string>,
            kc::Field<k::env_aa, std::string>,
            kc::Field<k::env_arch, std::string>,
            kc::Field<k::env_kv, std::string>,
            kc::Field<k::env_accept_keywords, std::string>,
            kc::Field<k::env_distdir, std::string>,
            kc::Field<k::env_portdir, std::string>,
            kc::Field<k::description_use, std::string>
                > EAPIEbuildEnvironmentVariables;

        /**
         * Information about a supported EAPI.
         *
         * \see EAPIData
         * \see EAPI
         * \ingroup grpeapi
         * \nosubgrouping
         */
        typedef kc::KeyedClass<
            kc::Field<k::package_dep_spec_parse_options, ELikePackageDepSpecOptions>,
            kc::Field<k::dependency_spec_tree_parse_options, erepository::DependencySpecTreeParseOptions>,
            kc::Field<k::iuse_flag_parse_options, IUseFlagParseOptions>,
            kc::Field<k::merger_options, MergerOptions>,
            kc::Field<k::breaks_portage, bool>,
            kc::Field<k::can_be_pbin, bool>,
            kc::Field<k::ebuild_options, const EAPIEbuildOptions>,
            kc::Field<k::ebuild_phases, const EAPIEbuildPhases>,
            kc::Field<k::ebuild_metadata_variables, const EAPIEbuildMetadataVariables>,
            kc::Field<k::ebuild_environment_variables, const EAPIEbuildEnvironmentVariables>,
            kc::Field<k::uri_labels, const EAPILabels>,
            kc::Field<k::dependency_labels, const EAPILabels>,
            kc::Field<k::pipe_commands, EAPIPipeCommands>,
            kc::Field<k::tools_options, const EAPIToolsOptions>
                > SupportedEAPI;

        /**
         * Information about an EAPI.
         *
         * \see EAPIData
         * \ingroup grpeapi
         * \nosubgrouping
         */
        typedef kc::KeyedClass<
            kc::Field<k::name, std::string>,
            kc::Field<k::exported_name, std::string>,
            kc::Field<k::supported, std::tr1::shared_ptr<const SupportedEAPI> >
                > EAPI;

    }
}

#endif
