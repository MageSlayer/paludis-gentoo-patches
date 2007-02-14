/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_PORTAGE_PORTAGE_REPOSITORY_METADATA_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_PORTAGE_PORTAGE_REPOSITORY_METADATA_HH 1

#include <paludis/name.hh>
#include <paludis/repository.hh>
#include <paludis/version_spec.hh>
#include <paludis/version_metadata.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/virtual_constructor.hh>
#include <paludis/repositories/gentoo/portage_repository_profile.hh>
#include <paludis/repositories/gentoo/portage_repository_params.hh>
#include <string>

/** \file
 * Declaration for the PortageRepositoryEntries class.
 *
 * \ingroup grpportagerepository
 */

namespace paludis
{
    class PortageRepository;
    class Environment;

    /**
     * Handle entries (for example, ebuilds) in a PortageRepository.
     *
     * \ingroup grpportagerepository
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE PortageRepositoryEntries
    {
        private:
            const std::string _ext;

        protected:
            ///\name Basic operations
            ///\{

            /// Constructor, with our file extension
            PortageRepositoryEntries(const std::string & ext);

            ///\}

        public:
            ///\name Basic operations
            ///\{

            virtual ~PortageRepositoryEntries();

            ///\}

            /**
             * Return our file extension, including the dot.
             */
            std::string file_extension() const
            {
                return _ext;
            }

            /**
             * Generate version metadata.
             */
            virtual std::tr1::shared_ptr<VersionMetadata> generate_version_metadata(const QualifiedPackageName &,
                    const VersionSpec &) const = 0;

            /**
             * Fetch an environment variable.
             */
            virtual std::string get_environment_variable(const QualifiedPackageName &,
                    const VersionSpec &, const std::string & var,
                    std::tr1::shared_ptr<const PortageRepositoryProfile>) const = 0;

            virtual void install(const QualifiedPackageName &, const VersionSpec &,
                    const InstallOptions &, std::tr1::shared_ptr<const PortageRepositoryProfile>) const = 0;

            virtual void merge(const MergeOptions &) = 0;
    };

    /**
     * Thrown if a repository of the specified type does not exist.
     *
     * \ingroup grpexceptions
     * \ingroup grprepository
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE NoSuchPortageRepositoryEntriesType : public ConfigurationError
    {
        public:
            /**
             * Constructor.
             */
            NoSuchPortageRepositoryEntriesType(const std::string & format) throw ();
    };

    /**
     * Virtual constructor for PortageRepositoryEntries.
     *
     * \ingroup grprepository
     */
    class PortageRepositoryEntriesMaker :
        public VirtualConstructor<std::string,
            std::tr1::shared_ptr<PortageRepositoryEntries> (*) (const Environment * const, PortageRepository * const,
                    const PortageRepositoryParams &),
            virtual_constructor_not_found::ThrowException<NoSuchPortageRepositoryEntriesType> >,
        public InstantiationPolicy<PortageRepositoryEntriesMaker, instantiation_method::SingletonTag>
    {
        friend class InstantiationPolicy<PortageRepositoryEntriesMaker, instantiation_method::SingletonTag>;

        private:
            PortageRepositoryEntriesMaker();
    };

}

#endif
