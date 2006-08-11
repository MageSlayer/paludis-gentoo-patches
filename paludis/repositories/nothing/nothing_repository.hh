/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#ifndef PALUDIS_GUARD_PALUDIS_NOTHING_REPOSITORY_HH
#define PALUDIS_GUARD_PALUDIS_NOTHING_REPOSITORY_HH 1

#include <paludis/repository.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/fs_entry.hh>

/** \file
 * Declarations for the NothingRepository class.
 *
 * \ingroup grpnothingrepository
 */

namespace paludis
{
    class PackageDatabase;

#include <paludis/repositories/nothing/nothing_repository-sr.hh>

    /**
     * A NothingRepository is a Repository that has no content, but can be
     * synced along with other repositories.
     *
     * \ingroup grpnothingrepository
     */
    class NothingRepository :
        public Repository,
        public RepositorySyncableInterface,
        private PrivateImplementationPattern<NothingRepository>
    {
        protected:
            virtual bool do_has_category_named(const CategoryNamePart &) const;

            virtual bool do_has_package_named(const QualifiedPackageName &) const;

            virtual CategoryNamePartCollection::ConstPointer do_category_names() const;

            virtual QualifiedPackageNameCollection::ConstPointer do_package_names(
                    const CategoryNamePart &) const;

            virtual VersionSpecCollection::ConstPointer do_version_specs(
                    const QualifiedPackageName &) const;

            virtual bool do_has_version(const QualifiedPackageName &, const VersionSpec &) const;

            virtual VersionMetadata::ConstPointer do_version_metadata(
                    const QualifiedPackageName &,
                    const VersionSpec &) const;

            virtual bool do_is_licence(const std::string &) const;

            virtual bool do_sync() const;

        public:
            /**
             * Constructor.
             */
            NothingRepository(const NothingRepositoryParams &);

            /**
             * Virtual constructor.
             */
            static CountedPtr<Repository> make_nothing_repository(
                    const Environment * const env,
                    const PackageDatabase * const db,
                    AssociativeCollection<std::string, std::string>::ConstPointer);

            /**
             * Destructor.
             */
            ~NothingRepository();

            virtual void invalidate() const;

            virtual ProvideMapIterator begin_provide_map() const;

            virtual ProvideMapIterator end_provide_map() const;

            typedef CountedPtr<NothingRepository, count_policy::InternalCountTag> Pointer;
            typedef CountedPtr<const NothingRepository, count_policy::InternalCountTag> ConstPointer;
    };

    /**
     * Thrown if invalid parameters are provided for
     * NothingRepository::make_nothing_repository.
     *
     * \ingroup grpexceptions
     * \ingroup grpnothingrepository
     */
    class NothingRepositoryConfigurationError : public ConfigurationError
    {
        public:
            /**
             * Constructor.
             */
            NothingRepositoryConfigurationError(const std::string & msg) throw ();
    };

    /**
     * Register NothingRepository.
     */
    static const RepositoryMaker::RegisterMaker register_nothing_repository(
            "nothing", &NothingRepository::make_nothing_repository);

}


#endif
