/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2010 Ciaran McCreesh
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
#include <paludis/util/named_value.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/singleton.hh>
#include <memory>

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
        typedef Name<struct concept_keyword_name> concept_keyword;
        typedef Name<struct concept_license_name> concept_license;
        typedef Name<struct concept_use_name> concept_use;
        typedef Name<struct default_environment_name> default_environment;
        typedef Name<struct extra_data_dir_name> extra_data_dir;
        typedef Name<struct fallback_environment_name> fallback_environment;
        typedef Name<struct name_name> name;
        typedef Name<struct paludis_package_name> paludis_package;
        typedef Name<struct support_old_style_virtuals_name> support_old_style_virtuals;
    }

    /**
     * Information about a distribution.
     *
     * The Distribution::config_dir key points to a directory that can be used
     * by submodules to store their own configuration.
     *
     * \see DistributionData
     * \ingroup g_distribution
     * \since 0.30
     * \nosubgrouping
     */
    struct Distribution
    {
        NamedValue<n::concept_keyword, std::string> concept_keyword;
        NamedValue<n::concept_license, std::string> concept_license;
        NamedValue<n::concept_use, std::string> concept_use;
        NamedValue<n::default_environment, std::string> default_environment;
        NamedValue<n::extra_data_dir, FSEntry> extra_data_dir;
        NamedValue<n::fallback_environment, std::string> fallback_environment;
        NamedValue<n::name, std::string> name;
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
        public Singleton<DistributionData>
    {
        friend class Singleton<DistributionData>;

        private:
            DistributionData();
            ~DistributionData();

        public:
            /**
             * Fetch a distribution from a named string.
             */
            std::shared_ptr<const Distribution> distribution_from_string(const std::string &) const;
    };

    /**
     * Fetch module-specific information about a distribution.
     *
     * Various modules provide typedefs for instantiations of this template,
     * allowing access to additional information abotu a distribution.
     *
     * \ingroup g_distribution
     * \since 0.30
     */
    template <typename Data_>
    class PALUDIS_VISIBLE ExtraDistributionData :
        private PrivateImplementationPattern<ExtraDistributionData<Data_> >,
        public Singleton<ExtraDistributionData<Data_> >
    {
        friend class Singleton<ExtraDistributionData<Data_> >;

        private:
            ExtraDistributionData();
            ~ExtraDistributionData();

        public:
            /**
             * Fetch our data from a given distribution.
             */
            const std::shared_ptr<const Data_> data_from_distribution(
                    const Distribution &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    extern template class Singleton<DistributionData>;
}

#endif
