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

#ifndef PALUDIS_GUARD_PALUDIS_DISTRIBUTION_HH
#define PALUDIS_GUARD_PALUDIS_DISTRIBUTION_HH 1

#include <paludis/distribution-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/named_value.hh>
#include <tr1/memory>

/** \file
 * Declarations for distributions.
 *
 * \ingroup g_distribution
 *
 * \section Examples
 *
 * - None at this time. The Distribution classes are of little direct use to
 *   clients; they are mainly used by Repository and Environment implementations.
 */

namespace paludis
{
    namespace n
    {
        struct concept_keyword;
        struct concept_use;
        struct default_ebuild_builddir;
        struct default_ebuild_distdir;
        struct default_ebuild_eapi_when_unknown;
        struct default_ebuild_eapi_when_unspecified;
        struct default_ebuild_layout;
        struct default_ebuild_names_cache;
        struct default_ebuild_profile_eapi;
        struct default_ebuild_write_cache;
        struct default_environment;
        struct default_vdb_names_cache;
        struct default_vdb_provides_cache;
        struct fallback_environment;
        struct paludis_environment_keywords_conf_filename;
        struct paludis_environment_use_conf_filename;
        struct paludis_package;
        struct support_old_style_virtuals;
    }

    /**
     * Information about a distribution.
     *
     * \see DistributionData
     * \ingroup g_distribution
     * \since 0.30
     * \nosubgrouping
     */
    struct Distribution
    {
        NamedValue<n::concept_keyword, std::string> concept_keyword;
        NamedValue<n::concept_use, std::string> concept_use;
        NamedValue<n::default_ebuild_builddir, std::string> default_ebuild_builddir;
        NamedValue<n::default_ebuild_distdir, std::string> default_ebuild_distdir;
        NamedValue<n::default_ebuild_eapi_when_unknown, std::string> default_ebuild_eapi_when_unknown;
        NamedValue<n::default_ebuild_eapi_when_unspecified, std::string> default_ebuild_eapi_when_unspecified;
        NamedValue<n::default_ebuild_layout, std::string> default_ebuild_layout;
        NamedValue<n::default_ebuild_names_cache, std::string> default_ebuild_names_cache;
        NamedValue<n::default_ebuild_profile_eapi, std::string> default_ebuild_profile_eapi;
        NamedValue<n::default_ebuild_write_cache, std::string> default_ebuild_write_cache;
        NamedValue<n::default_environment, std::string> default_environment;
        NamedValue<n::default_vdb_names_cache, std::string> default_vdb_names_cache;
        NamedValue<n::default_vdb_provides_cache, std::string> default_vdb_provides_cache;
        NamedValue<n::fallback_environment, std::string> fallback_environment;
        NamedValue<n::paludis_environment_keywords_conf_filename, std::string> paludis_environment_keywords_conf_filename;
        NamedValue<n::paludis_environment_use_conf_filename, std::string> paludis_environment_use_conf_filename;
        NamedValue<n::paludis_package, std::string> paludis_package;
        NamedValue<n::support_old_style_virtuals, bool> support_old_style_virtuals;
    };

    /**
     * Thrown if an invalid distribution file is encountered.
     *
     * \ingroup g_distribution
     * \ingroup g_exceptions
     * \since 0.26
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE DistributionConfigurationError :
        public ConfigurationError
    {
        public:
            ///\name Basic operations
            ///\{

            DistributionConfigurationError(const std::string &) throw ();

            ///\}
    };

    /**
     * Fetch information about a distribution.
     *
     * Paludis avoids hardcoding certain distribution-related information to
     * make things easier for other distributions. Instead, DistributionData
     * is used to fetch a Distribution class instance. The
     * distribution_from_string method is almost always called with the return
     * value of Environment::distribution as its parameter.
     *
     * \ingroup g_distribution
     * \since 0.26
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE DistributionData :
        private PrivateImplementationPattern<DistributionData>,
        public InstantiationPolicy<DistributionData, instantiation_method::SingletonTag>
    {
        friend class InstantiationPolicy<DistributionData, instantiation_method::SingletonTag>;

        private:
            DistributionData();
            ~DistributionData();

        public:
            /**
             * Fetch a distribution from a named string.
             */
            std::tr1::shared_ptr<const Distribution> distribution_from_string(const std::string &) const;
    };
}

#endif
