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
     * A FakeRepositoryBase is a Repository subclass whose subclasses are used for
     * various test cases.
     *
     * \see FakeRepository
     * \see FakeInstalledRepository
     * \ingroup grpfakerepository
     */
    class FakeRepositoryBase :
        public Repository,
        public RepositoryMaskInterface,
        public RepositoryUseInterface,
        private PrivateImplementationPattern<FakeRepositoryBase>
    {
        protected:
            virtual bool do_has_category_named(const CategoryNamePart &) const;

            virtual bool do_has_package_named(const QualifiedPackageName &) const;

            virtual CategoryNamePartCollection::ConstPointer do_category_names() const;

            virtual QualifiedPackageNameCollection::ConstPointer do_package_names(
                    const CategoryNamePart &) const;

            virtual VersionSpecCollection::ConstPointer do_version_specs(
                    const QualifiedPackageName &) const;

            virtual bool do_has_version(const QualifiedPackageName &,
                    const VersionSpec &) const;

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

            virtual UseFlagNameCollection::ConstPointer do_arch_flags() const;

            virtual bool do_is_expand_flag(const UseFlagName &) const;
            virtual bool do_is_expand_hidden_flag(const UseFlagName &) const;
            virtual std::string::size_type do_expand_flag_delim_pos(const UseFlagName &) const;

            virtual bool do_is_licence(const std::string &) const;

        protected:
            /**
             * Constructor.
             */
            FakeRepositoryBase(const RepositoryName & name, const RepositoryCapabilities & caps);

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
            VersionMetadata::Pointer add_version(
                    const QualifiedPackageName &, const VersionSpec &);

            /**
             * Add a version, and a package and category if necessary, and set some
             * default values for its metadata, and return said metadata (convenience
             * overload taking strings).
             */
            VersionMetadata::Pointer add_version(
                    const std::string & c, const std::string & p, const std::string & v)
            {
                return add_version(CategoryNamePart(c) + PackageNamePart(p), VersionSpec(v));
            }

            /**
             * A non-constant smart pointer to ourself.
             */
            typedef CountedPtr<FakeRepositoryBase, count_policy::InternalCountTag> Pointer;

            /**
             * A constant smart pointer to ourself.
             */
            typedef CountedPtr<const FakeRepositoryBase, count_policy::InternalCountTag> ConstPointer;

            virtual void invalidate() const;
    };
}


#endif
