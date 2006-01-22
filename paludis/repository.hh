/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORY_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORY_HH 1

#include <paludis/attributes.hh>
#include <paludis/repository_name.hh>
#include <paludis/category_name_part.hh>
#include <paludis/package_name_part.hh>
#include <paludis/category_name_part_collection.hh>
#include <paludis/qualified_package_name_collection.hh>
#include <paludis/counted_ptr.hh>
#include <paludis/version_metadata.hh>
#include <paludis/version_spec.hh>
#include <paludis/use_flag_state.hh>
#include <map>
#include <string>

/** \file
 * Declarations for the Repository class.
 */

namespace paludis
{
    /**
     * A Repository provides a representation of a physical repository to a
     * PackageDatabase.
     */
    class Repository :
        private InstantiationPolicy<Repository, instantiation_method::NonCopyableTag>,
        public InternalCounted<Repository>
    {
        private:
            const RepositoryName _name;

        protected:
            std::map<std::string, std::string> _info;

            /**
             * Constructor.
             */
            Repository(const RepositoryName &);

            /**
             * Override in descendents: fetch the metadata.
             */
            virtual VersionMetadata::ConstPointer do_version_metadata(
                    const CategoryNamePart &, const PackageNamePart &,
                    const VersionSpec &) const = 0;

            /**
             * Override in descendents: check for a version.
             */
            virtual bool do_has_version(const CategoryNamePart &,
                    const PackageNamePart &, const VersionSpec &) const = 0;

            /**
             * Override in descendents: fetch version specs.
             */
            virtual VersionSpecCollection::ConstPointer do_version_specs(
                    const QualifiedPackageName &) const = 0;

            /**
             * Override in descendents: fetch package names.
             */
            virtual QualifiedPackageNameCollection::ConstPointer do_package_names(
                    const CategoryNamePart &) const = 0;

            /**
             * Override in descendents: fetch category names.
             */
            virtual CategoryNamePartCollection::ConstPointer do_category_names() const = 0;

            /**
             * Override in descendents: check for a package.
             */
            virtual bool do_has_package_named(const CategoryNamePart &,
                    const PackageNamePart &) const = 0;

            /**
             * Override in descendents: check for a category.
             */
            virtual bool do_has_category_named(const CategoryNamePart &) const = 0;

            /**
             * Override in descendents: check for a mask.
             */
            virtual bool do_query_repository_masks(const CategoryNamePart &,
                    const PackageNamePart &, const VersionSpec &) const = 0;

            /**
             * Override in descendents: check for a mask.
             */
            virtual bool do_query_profile_masks(const CategoryNamePart &,
                    const PackageNamePart &, const VersionSpec &) const = 0;

            /**
             * Override in descendents: get use.
             */
            virtual UseFlagState do_query_use(const UseFlagName &) const = 0;

            /**
             * Override in descendents: get use mask.
             */
            virtual bool do_query_use_mask(const UseFlagName &) const = 0;

        public:
            /**
             * Destructor.
             */
            virtual ~Repository();

            /**
             * Return our name.
             */
            const RepositoryName & name() const PALUDIS_ATTRIBUTE((nothrow));

            /**
             * Do we have a category with the given name?
             */
            bool has_category_named(const CategoryNamePart & c) const
            {
                return do_has_category_named(c);
            }

            /**
             * Do we have a package in the given category with the given name?
             */
            bool has_package_named(const CategoryNamePart & c,
                    const PackageNamePart & p) const
            {
                return do_has_package_named(c, p);
            }

            /**
             * Do we have a package in the given category with the given name?
             * (Override for QualifiedPackageName).
             */
            bool has_package_named(const QualifiedPackageName & q) const
            {
                return has_package_named(q.get<qpn_category>(), q.get<qpn_package>());
            }

            /**
             * Fetch our category names.
             */
            CategoryNamePartCollection::ConstPointer category_names() const
            {
                return do_category_names();
            }

            /**
             * Fetch our package names.
             */
            QualifiedPackageNameCollection::ConstPointer package_names(
                    const CategoryNamePart & c) const
            {
                return do_package_names(c);
            }

            /**
             * Fetch our versions.
             */
            VersionSpecCollection::ConstPointer version_specs(
                    const QualifiedPackageName & p) const
            {
                return do_version_specs(p);
            }

            /**
             * Fetch our versions (override for split name).
             */
            VersionSpecCollection::ConstPointer version_specs(
                    const CategoryNamePart & c, const PackageNamePart & p) const
            {
                return version_specs(QualifiedPackageName(c, p));
            }

            /**
             * Do we have a version spec?
             */
            bool has_version(const CategoryNamePart & c,
                    const PackageNamePart & p, const VersionSpec & v) const
            {
                return do_has_version(c, p, v);
            }

            /**
             * Do we have a version spec? (Override for QualifiedPackageName).
             */
            bool has_version(const QualifiedPackageName & q, const VersionSpec & v) const
            {
                return has_version(q.get<qpn_category>(), q.get<qpn_package>(), v);
            }

            /**
             * Fetch metadata.
             */
            VersionMetadata::ConstPointer version_metadata(
                    const CategoryNamePart & c, const PackageNamePart & p,
                    const VersionSpec & v) const
            {
                return do_version_metadata(c, p, v);
            }

            /**
             * Fetch metadata (override for QualifiedPackageName).
             */
            VersionMetadata::ConstPointer version_metadata(
                    const QualifiedPackageName & q,
                    const VersionSpec & v) const
            {
                return version_metadata(q.get<qpn_category>(), q.get<qpn_package>(), v);
            }

            /**
             * Query repository masks.
             */
            bool query_repository_masks(const CategoryNamePart & c, const PackageNamePart & p,
                    const VersionSpec & v) const
            {
                return do_query_repository_masks(c, p, v);
            }

            /**
             * Query repository masks (override for QualifiedPackageName).
             */
            bool query_repository_masks(const QualifiedPackageName & q, const VersionSpec & v) const
            {
                return do_query_repository_masks(q.get<qpn_category>(), q.get<qpn_package>(), v);
            }

            /**
             * Query profile masks.
             */
            bool query_profile_masks(const CategoryNamePart & c, const PackageNamePart & p,
                    const VersionSpec & v) const
            {
                return do_query_profile_masks(c, p, v);
            }

            /**
             * Query profile masks (override for QualifiedPackageName).
             */
            bool query_profile_masks(const QualifiedPackageName & q, const VersionSpec & v) const
            {
                return do_query_profile_masks(q.get<qpn_category>(), q.get<qpn_package>(), v);
            }

            typedef std::map<std::string, std::string>::const_iterator InfoIterator;

            InfoIterator begin_info() const
            {
                return _info.begin();
            }

            InfoIterator end_info() const
            {
                return _info.end();
            }

            UseFlagState query_use(const UseFlagName & u) const
            {
                if (do_query_use_mask(u))
                    return use_disabled;
                else
                    return do_query_use(u);
            }

            bool query_use_mask(const UseFlagName & u) const
            {
                return do_query_use_mask(u);
            }
    };
}

#endif
