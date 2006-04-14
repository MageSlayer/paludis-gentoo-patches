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

#ifndef PALUDIS_GUARD_PALUDIS_FAKE_REPOSITORY_HH
#define PALUDIS_GUARD_PALUDIS_FAKE_REPOSITORY_HH 1

#include <paludis/repository.hh>
#include <paludis/util/private_implementation_pattern.hh>

/** \file
 * Declarations for the FakeRepository class.
 *
 * \ingroup Database
 */

namespace paludis
{
    /**
     * A FakeRepository is a Repository subclass that is used for
     * various test cases.
     *
     * \ingroup Database
     */
    class FakeRepository : public Repository,
                           private PrivateImplementationPattern<FakeRepository>
    {
        protected:
            virtual bool do_has_category_named(const CategoryNamePart &) const;

            virtual bool do_has_package_named(const CategoryNamePart &,
                    const PackageNamePart &) const;

            virtual CategoryNamePartCollection::ConstPointer do_category_names() const;

            virtual QualifiedPackageNameCollection::ConstPointer do_package_names(
                    const CategoryNamePart &) const;

            virtual VersionSpecCollection::ConstPointer do_version_specs(
                    const QualifiedPackageName &) const;

            virtual bool do_has_version(const CategoryNamePart &,
                    const PackageNamePart &, const VersionSpec &) const;

            virtual VersionMetadata::ConstPointer do_version_metadata(
                    const CategoryNamePart &, const PackageNamePart &,
                    const VersionSpec &) const;

            virtual bool do_query_repository_masks(const CategoryNamePart &,
                    const PackageNamePart &, const VersionSpec &) const;

            virtual bool do_query_profile_masks(const CategoryNamePart &,
                    const PackageNamePart &, const VersionSpec &) const;

            virtual UseFlagState do_query_use(const UseFlagName &) const;

            virtual bool do_query_use_mask(const UseFlagName &) const;

            virtual bool do_is_arch_flag(const UseFlagName &) const;

            virtual bool do_is_expand_flag(const UseFlagName &) const;

            virtual bool do_is_licence(const std::string &) const;

            virtual bool do_is_mirror(const std::string &) const;

            virtual void do_install(const QualifiedPackageName &, const VersionSpec &,
                    const InstallOptions &) const;

            virtual void do_uninstall(const QualifiedPackageName &, const VersionSpec &,
                    const InstallOptions &) const;

            virtual DepAtom::Pointer do_package_set(const std::string & s) const;

            virtual bool do_sync() const;

        public:
            /**
             * Constructor.
             */
            FakeRepository(const RepositoryName & name);

            /**
             * Destructor.
             */

            ~FakeRepository();

            /**
             * Add a category.
             */
            void add_category(const CategoryNamePart &);

            /**
             * Add a package, and a category if necessary.
             */
            void add_package(const CategoryNamePart &, const PackageNamePart &);

            /**
             * Add a version, and a package and category if necessary, and set some
             * default values for its metadata, and return said metadata.
             */
            VersionMetadata::Pointer add_version(
                    const CategoryNamePart &, const PackageNamePart &, const VersionSpec &);

            /**
             * Add a version, and a package and category if necessary, and set some
             * default values for its metadata, and return said metadata (convenience
             * overload taking strings).
             */
            VersionMetadata::Pointer add_version(
                    const std::string & c, const std::string & p, const std::string & v)
            {
                return add_version(CategoryNamePart(c), PackageNamePart(p), VersionSpec(v));
            }

            typedef CountedPtr<FakeRepository, count_policy::InternalCountTag> Pointer;
            typedef CountedPtr<const FakeRepository, count_policy::InternalCountTag> ConstPointer;

            virtual bool installed() const
            {
                return false;
            }

            virtual void invalidate() const;

            virtual ProvideMapIterator begin_provide_map() const;

            virtual ProvideMapIterator end_provide_map() const;

            virtual void remove_from_world(const QualifiedPackageName &) const
            {
            }

            virtual void add_to_world(const QualifiedPackageName &) const
            {
            }
    };
}


#endif
