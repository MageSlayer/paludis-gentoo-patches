/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#include <map>
#include <paludis/repository.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/smart_record.hh>
#include <string>

/** \file
 * Declaration for the PortageRepository class.
 *
 * \ingroup grpportagerepository
 */

namespace paludis
{
    class PackageDatabase;

    /**
     * Keys for PortageRepositoryParams.
     *
     * \see PortageRepositoryParams
     * \ingroup grpportagerepository
     */
    enum PortageRepositoryParamsKeys
    {
        prpk_environment,
        prpk_package_database,
        prpk_location,
        prpk_profiles,
        prpk_cache,
        prpk_distdir,
        prpk_eclassdirs,
        prpk_setsdir,
        prpk_securitydir,
        prpk_newsdir,
        prpk_sync,
        prpk_sync_exclude,
        prpk_root,
        prpk_buildroot,
        last_prpk
    };

    /**
     * Tag for PortageRepositoryParams.
     *
     * \see PortageRepositoryParams
     * \ingroup grpportagerepository
     */
    struct PortageRepositoryParamsTag :
        SmartRecordTag<comparison_mode::NoComparisonTag, void>,
        SmartRecordKeys<PortageRepositoryParamsKeys, last_prpk>,
        SmartRecordKey<prpk_environment, const Environment *>,
        SmartRecordKey<prpk_package_database, const PackageDatabase *>,
        SmartRecordKey<prpk_location, const FSEntry>,
        SmartRecordKey<prpk_profiles, FSEntryCollection::Pointer>,
        SmartRecordKey<prpk_cache, const FSEntry>,
        SmartRecordKey<prpk_distdir, const FSEntry>,
        SmartRecordKey<prpk_eclassdirs, FSEntryCollection::Pointer>,
        SmartRecordKey<prpk_setsdir, const FSEntry>,
        SmartRecordKey<prpk_securitydir, const FSEntry>,
        SmartRecordKey<prpk_newsdir, const FSEntry>,
        SmartRecordKey<prpk_sync, const std::string>,
        SmartRecordKey<prpk_sync_exclude, const std::string>,
        SmartRecordKey<prpk_root, const FSEntry>,
        SmartRecordKey<prpk_buildroot, const FSEntry>
    {
    };

    /**
     * Constructor parameters for PortageRepository.
     *
     * \see PortageRepository
     * \ingroup grpportagerepository
     */
    typedef MakeSmartRecord<PortageRepositoryParamsTag>::Type PortageRepositoryParams;

    /**
     * A PortageRepository is a Repository that handles the layout used by
     * Portage for the main Gentoo tree.
     *
     * \ingroup grpportagerepository
     */
    class PortageRepository :
        public Repository,
        public Repository::MaskInterface,
        public Repository::UseInterface,
        public Repository::InstallableInterface,
        public Repository::SyncableInterface,
        public Repository::NewsInterface,
        public Repository::SetsInterface,
        public Repository::EnvironmentVariableInterface,
        private PrivateImplementationPattern<PortageRepository>
    {
        private:
            void need_category_names() const;
            void need_version_names(const QualifiedPackageName &) const;
            void need_virtual_names() const;
            PackageDatabaseEntryCollection::Iterator find_best(PackageDatabaseEntryCollection & c,
                    const PackageDatabaseEntry & e) const;
            DepAtom::Pointer do_security_set(const PackageSetOptions & o) const;

        protected:
            /**
             * Try to get the repository name for a particular repository.
             */
            static RepositoryName fetch_repo_name(const std::string & location);

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

            virtual bool do_query_repository_masks(const QualifiedPackageName &,
                    const VersionSpec &) const;

            virtual bool do_query_profile_masks(const QualifiedPackageName &,
                    const VersionSpec &) const;

            virtual UseFlagState do_query_use(const UseFlagName &, const PackageDatabaseEntry *) const;

            virtual bool do_query_use_mask(const UseFlagName &, const PackageDatabaseEntry *) const;

            virtual bool do_query_use_force(const UseFlagName &, const PackageDatabaseEntry *) const;

            virtual bool do_is_arch_flag(const UseFlagName &) const;

            virtual bool do_is_expand_flag(const UseFlagName &) const;

            virtual bool do_is_expand_hidden_flag(const UseFlagName &) const;

            virtual std::string::size_type do_expand_flag_delim_pos(const UseFlagName &) const;

            virtual bool do_is_licence(const std::string &) const;

            virtual bool do_is_mirror(const std::string &) const;

            virtual void do_install(const QualifiedPackageName &, const VersionSpec &,
                    const InstallOptions &) const;

            virtual DepAtom::Pointer do_package_set(const std::string &, const PackageSetOptions &) const;

            virtual bool do_sync() const;

        public:
            virtual RepositoryInfo::ConstPointer info(bool verbose) const;

            /**
             * Constructor.
             */
            PortageRepository(const PortageRepositoryParams &);

            /**
             * Virtual constructor.
             */
            static CountedPtr<Repository> make_portage_repository(
                    const Environment * const env,
                    const PackageDatabase * const db,
                    const std::map<std::string, std::string> &);

            /**
             * Destructor.
             */
            ~PortageRepository();

            virtual bool installed() const
            {
                return false;
            }

            virtual void invalidate() const;

            virtual ProvideMapIterator begin_provide_map() const;

            virtual ProvideMapIterator end_provide_map() const;

            virtual void update_news() const;

            virtual std::string get_environment_variable(
                    const PackageDatabaseEntry & for_package,
                    const std::string & var) const;

            typedef CountedPtr<PortageRepository, count_policy::InternalCountTag> Pointer;
            typedef CountedPtr<const PortageRepository, count_policy::InternalCountTag> ConstPointer;
    };

    /**
     * Class to convert AdvisoryFile lines to PackageDepAtom(s).
     */
    class AdvisoryVisitor :
        private InstantiationPolicy<AdvisoryVisitor, instantiation_method::NonCopyableTag>,
        public DepAtomVisitorTypes::ConstVisitor
    {
        private:
            const Environment * const _env;

            mutable const CompositeDepAtom & _a;

            mutable std::vector<const PackageDepAtom *> _atoms;

        protected:
            ///\name Visit methods
            ///{
            void visit(const AllDepAtom *);
            void visit(const AnyDepAtom *) PALUDIS_ATTRIBUTE((noreturn));
            void visit(const UseDepAtom *);
            void visit(const PlainTextDepAtom *);
            void visit(const PackageDepAtom *);
            void visit(const BlockDepAtom *);
            ///}

        public:
            /**
             * Constructor.
             */
            AdvisoryVisitor(const Environment * const env, const CompositeDepAtom & a);

            /**
             * Destructor.
             */
            ~AdvisoryVisitor()
            {
            }

            /**
             * Iterate over our dep atoms.
             */
            typedef std::vector<const PackageDepAtom *>::iterator Iterator;

            /**
             * Grab element by index.
             */
            const PackageDepAtom * at(std::vector<const PackageDepAtom *>::size_type n) const
            {
                return _atoms[n];
            }

            /**
             * Return the number of atoms.
             */
            std::vector<const PackageDepAtom *>::size_type size() const
            {
                return _atoms.size();
            }
    };

    /**
     * Thrown if invalid parameters are provided for
     * PortageRepository::make_portage_repository.
     *
     * \ingroup grpexceptions
     * \ingroup grpportagerepository
     */
    class PortageRepositoryConfigurationError : public ConfigurationError
    {
        public:
            /**
             * Constructor.
             */
            PortageRepositoryConfigurationError(const std::string & msg) throw ();
    };

    /**
     * Register PortageRepository.
     */
    static const RepositoryMaker::RegisterMaker register_portage_repository(
            "portage", &PortageRepository::make_portage_repository);

}

#endif
