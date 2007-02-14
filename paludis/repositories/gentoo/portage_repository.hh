/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_PORTAGE_REPOSITORY_HH
#define PALUDIS_GUARD_PALUDIS_PORTAGE_REPOSITORY_HH 1

#include <paludis/repository.hh>
#include <paludis/package_database.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/repositories/gentoo/portage_repository_params.hh>
#include <paludis/repositories/gentoo/portage_repository_profile.hh>
#include <string>

/** \file
 * Declaration for the PortageRepository class.
 *
 * \ingroup grpportagerepository
 */

namespace paludis
{
    class PortageRepositoryProfile;
    class PortageRepositoryNews;

#include <paludis/repositories/gentoo/portage_repository-sr.hh>

    /**
     * A PortageRepository is a Repository that handles the layout used by
     * Portage for the main Gentoo tree.
     *
     * \ingroup grpportagerepository
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE PortageRepository :
        public Repository,
        public RepositoryMaskInterface,
        public RepositoryUseInterface,
        public RepositoryInstallableInterface,
        public RepositorySyncableInterface,
        public RepositoryNewsInterface,
        public RepositorySetsInterface,
        public RepositoryEnvironmentVariableInterface,
        public RepositoryMirrorsInterface,
        public RepositoryVirtualsInterface,
        public RepositoryLicensesInterface,
        private PrivateImplementationPattern<PortageRepository>
    {
        private:
            void need_category_names() const;
            void need_version_names(const QualifiedPackageName &) const;
            void need_mirrors() const;
            PackageDatabaseEntryCollection::Iterator find_best(PackageDatabaseEntryCollection & c,
                    const PackageDatabaseEntry & e) const;

        protected:
            /**
             * Try to get the repository name for a particular repository.
             */
            static RepositoryName fetch_repo_name(const std::string & location);

            virtual bool do_has_category_named(const CategoryNamePart &) const;

            virtual bool do_has_package_named(const QualifiedPackageName &) const;

            virtual std::tr1::shared_ptr<const CategoryNamePartCollection> do_category_names() const;

            std::tr1::shared_ptr<const CategoryNamePartCollection> do_category_names_containing_package(
                    const PackageNamePart & p) const;

            virtual std::tr1::shared_ptr<const QualifiedPackageNameCollection> do_package_names(
                    const CategoryNamePart &) const;

            virtual std::tr1::shared_ptr<const VersionSpecCollection> do_version_specs(
                    const QualifiedPackageName &) const;

            virtual bool do_has_version(const QualifiedPackageName &, const VersionSpec &) const;

            virtual std::tr1::shared_ptr<const VersionMetadata> do_version_metadata(
                    const QualifiedPackageName &,
                    const VersionSpec &) const;

            virtual bool do_query_repository_masks(const QualifiedPackageName &,
                    const VersionSpec &) const;

            virtual bool do_query_profile_masks(const QualifiedPackageName &,
                    const VersionSpec &) const;

            virtual std::tr1::shared_ptr<FSEntry> do_license_exists(
                    const std::string & license) const;

            virtual void do_install(const QualifiedPackageName &, const VersionSpec &,
                    const InstallOptions &) const;

            virtual std::tr1::shared_ptr<DepAtom> do_package_set(const SetName &) const;

            virtual std::tr1::shared_ptr<const SetsCollection> sets_list() const;

            virtual bool do_sync() const;

            virtual std::tr1::shared_ptr<const VirtualsCollection> virtual_packages() const;

            virtual std::tr1::shared_ptr<const VersionMetadata> virtual_package_version_metadata(
                    const RepositoryVirtualsEntry &, const VersionSpec & v) const;

            /* RepositoryUseInterface */

            virtual UseFlagState do_query_use(const UseFlagName &, const PackageDatabaseEntry *) const;
            virtual bool do_query_use_mask(const UseFlagName &, const PackageDatabaseEntry *) const;
            virtual bool do_query_use_force(const UseFlagName &, const PackageDatabaseEntry *) const;
            virtual std::tr1::shared_ptr<const UseFlagNameCollection> do_arch_flags() const;
            virtual std::tr1::shared_ptr<const UseFlagNameCollection> do_use_expand_flags() const;
            virtual std::tr1::shared_ptr<const UseFlagNameCollection> do_use_expand_hidden_prefixes() const;
            virtual UseFlagName do_use_expand_name(const UseFlagName & u) const;
            virtual UseFlagName do_use_expand_value(const UseFlagName & u) const;
            virtual std::tr1::shared_ptr<const UseFlagNameCollection> do_use_expand_prefixes() const;
            virtual std::string do_describe_use_flag(const UseFlagName &,
                    const PackageDatabaseEntry * const) const;

            /* end of RepositoryUseInterface */

        public:
            virtual std::tr1::shared_ptr<const RepositoryInfo> info(bool verbose) const;

            /**
             * Constructor.
             */
            PortageRepository(const PortageRepositoryParams &);

            /**
             * Destructor.
             */
            ~PortageRepository();

            virtual void invalidate();
            virtual void regenerate_cache() const;

            virtual void update_news() const;

            virtual std::string get_environment_variable(
                    const PackageDatabaseEntry & for_package,
                    const std::string & var) const;

            virtual MirrorsIterator begin_mirrors(const std::string & s) const;
            virtual MirrorsIterator end_mirrors(const std::string & s) const;

            ///\name Information about PortageRepository
            ///\{

            std::string profile_variable(const std::string &) const;

            typedef libwrapiter::ForwardIterator<PortageRepository, std::pair<
                const QualifiedPackageName, std::tr1::shared_ptr<const PackageDepAtom> > > OurVirtualsIterator;

            const PortageRepositoryParams & params() const;

            ///\}

            ///\name Profile setting and querying functions
            ///\{

            typedef libwrapiter::ForwardIterator<PortageRepository,
                    const PortageRepositoryProfilesDescLine> ProfilesIterator;
            ProfilesIterator begin_profiles() const;
            ProfilesIterator end_profiles() const;

            ProfilesIterator find_profile(const FSEntry & location) const;
            void set_profile(const ProfilesIterator & iter);
            void set_profile_by_arch(const UseFlagName &);

            ///\}
    };
}

#endif
