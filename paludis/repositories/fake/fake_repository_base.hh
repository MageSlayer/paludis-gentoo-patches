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

#ifndef PALUDIS_GUARD_PALUDIS_FAKE_REPOSITORY_BASE_HH
#define PALUDIS_GUARD_PALUDIS_FAKE_REPOSITORY_BASE_HH 1

#include <paludis/repository.hh>
#include <paludis/util/private_implementation_pattern.hh>

/** \file
 * Declarations for the FakeRepositoryBase class.
 *
 * \ingroup grpfakerepository
 */

namespace paludis
{
    /**
     * FakeVersionMetadata is used by FakeRepository and
     * FakeInstalledRepository for version metadata.
     *
     * \see FakeInstalledRepository
     * \see FakeRepository
     * \ingroup grpfakerepository
     */
    class FakeVersionMetadata :
        public VersionMetadata,
        public VersionMetadataEbuildInterface,
        public VersionMetadataDepsInterface,
        public VersionMetadataLicenseInterface,
        public virtual VersionMetadataHasInterfaces
    {
        public:
            FakeVersionMetadata();
            virtual ~FakeVersionMetadata();

            virtual const VersionMetadata * version_metadata() const
            {
                return this;
            }
    };

    /**
     * FakeVirtualVersionMetadata is used by FakeRepository for virtual
     * version metadata.
     *
     * \see FakeRepository
     * \ingroup grpfakerepository
     */
    class FakeVirtualVersionMetadata :
        public VersionMetadata,
        public VersionMetadataDepsInterface,
        public VersionMetadataVirtualInterface,
        public virtual VersionMetadataHasInterfaces
    {
        public:
            ///\name Basic operations
            ///\{

            FakeVirtualVersionMetadata(const SlotName &, const PackageDatabaseEntry &);
            virtual ~FakeVirtualVersionMetadata();

            ///\}

            virtual const VersionMetadata * version_metadata() const
            {
                return this;
            }
    };

    /**
     * A FakeRepositoryBase is a Repository subclass whose subclasses are used for
     * various test cases.
     *
     * \see FakeRepository
     * \see FakeInstalledRepository
     * \ingroup grpfakerepository
     */
    class PALUDIS_VISIBLE FakeRepositoryBase :
        public Repository,
        public RepositoryMaskInterface,
        public RepositoryUseInterface,
        public RepositorySetsInterface,
        private PrivateImplementationPattern<FakeRepositoryBase>
    {
        protected:
            /* RepositoryUseInterface */

            virtual UseFlagState do_query_use(const UseFlagName &, const PackageDatabaseEntry &) const;
            virtual bool do_query_use_mask(const UseFlagName &, const PackageDatabaseEntry &) const;
            virtual bool do_query_use_force(const UseFlagName &, const PackageDatabaseEntry &) const;
            virtual std::tr1::shared_ptr<const UseFlagNameCollection> do_arch_flags() const;
            virtual std::tr1::shared_ptr<const UseFlagNameCollection> do_use_expand_flags() const;
            virtual std::tr1::shared_ptr<const UseFlagNameCollection> do_use_expand_hidden_prefixes() const;
            virtual std::tr1::shared_ptr<const UseFlagNameCollection> do_use_expand_prefixes() const;
            virtual std::string do_describe_use_flag(const UseFlagName &, const PackageDatabaseEntry &) const;

            /* end of RepositoryUseInterface */

            virtual bool do_has_category_named(const CategoryNamePart &) const;

            virtual bool do_has_package_named(const QualifiedPackageName &) const;

            virtual std::tr1::shared_ptr<const CategoryNamePartCollection> do_category_names() const;

            virtual std::tr1::shared_ptr<const QualifiedPackageNameCollection> do_package_names(
                    const CategoryNamePart &) const;

            virtual std::tr1::shared_ptr<const VersionSpecCollection> do_version_specs(
                    const QualifiedPackageName &) const;

            virtual bool do_has_version(const QualifiedPackageName &,
                    const VersionSpec &) const;

            virtual std::tr1::shared_ptr<const VersionMetadata> do_version_metadata(
                    const QualifiedPackageName &,
                    const VersionSpec &) const;

            virtual bool do_query_repository_masks(const QualifiedPackageName &,
                    const VersionSpec &) const;

            virtual bool do_query_profile_masks(const QualifiedPackageName &,
                    const VersionSpec &) const;

            virtual std::tr1::shared_ptr<DepSpec> do_package_set(const SetName & id) const;
            virtual std::tr1::shared_ptr<const SetNameCollection> sets_list() const;

        protected:
            /**
             * Constructor.
             */
            FakeRepositoryBase(const Environment * const env, const RepositoryName & name,
                    const RepositoryCapabilities & caps, const std::string &);

        public:
            /**
             * Destructor.
             */

            ~FakeRepositoryBase();

            /**
             * Add a category.
             */
            void add_category(const CategoryNamePart &);

            /**
             * Add a package, and a category if necessary.
             */
            void add_package(const QualifiedPackageName &);

            /**
             * Add a version, and a package and category if necessary, and set some
             * default values for its metadata, and return said metadata.
             */
            std::tr1::shared_ptr<VersionMetadata> add_version(
                    const QualifiedPackageName &, const VersionSpec &);

            /**
             * Add a version, and a package and category if necessary, and set some
             * default values for its metadata, and return said metadata (convenience
             * overload taking strings).
             */
            std::tr1::shared_ptr<VersionMetadata> add_version(
                    const std::string & c, const std::string & p, const std::string & v)
            {
                return add_version(CategoryNamePart(c) + PackageNamePart(p), VersionSpec(v));
            }

            /**
             * Add a package set.
             */
            void add_package_set(const SetName &, std::tr1::shared_ptr<DepSpec>);

            virtual void invalidate();

            /**
             * Fetch our associated environment.
             */
            const Environment * environment() const;
    };
}


#endif
