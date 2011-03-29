/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_PARTIALLY_MADE_PACKAGE_DEP_SPEC_HH
#define PALUDIS_GUARD_PALUDIS_PARTIALLY_MADE_PACKAGE_DEP_SPEC_HH 1

#include <paludis/partially_made_package_dep_spec-fwd.hh>
#include <paludis/util/pimp.hh>
#include <paludis/name-fwd.hh>
#include <paludis/dep_spec.hh>

namespace paludis
{
    /**
     * A PartiallyMadePackageDepSpec is returned by make_package_dep_spec()
     * and is used to incrementally build a PackageDepSpec.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     */
    class PALUDIS_VISIBLE PartiallyMadePackageDepSpec
    {
        private:
            Pimp<PartiallyMadePackageDepSpec> _imp;

        public:
            ///\name Basic operations
            ///\{

            PartiallyMadePackageDepSpec(const PartiallyMadePackageDepSpecOptions &);
            ~PartiallyMadePackageDepSpec();
            PartiallyMadePackageDepSpec(const PackageDepSpec &);
            PartiallyMadePackageDepSpec(const PartiallyMadePackageDepSpec &);

            ///\}

            /**
             * Set our package requirements, return ourself.
             */
            PartiallyMadePackageDepSpec & package(const QualifiedPackageName &);

            /**
             * Clear our package requirements, return ourself.
             *
             * \since 0.55
             */
            PartiallyMadePackageDepSpec & clear_package();

            /**
             * Set our slot requirements, return ourself.
             */
            PartiallyMadePackageDepSpec & slot_requirement(const std::shared_ptr<const SlotRequirement> &);

            /**
             * Clear our slot requirements, return ourself.
             *
             * \since 0.55
             */
            PartiallyMadePackageDepSpec & clear_slot_requirement();

            /**
             * Set our in-repository requirement, return ourself.
             */
            PartiallyMadePackageDepSpec & in_repository(const RepositoryName &);

            /**
             * Clear our in-repository requirement, return ourself.
             *
             * \since 0.55
             */
            PartiallyMadePackageDepSpec & clear_in_repository();

            /**
             * Set our from-repository requirement, return ourself.
             */
            PartiallyMadePackageDepSpec & from_repository(const RepositoryName &);

            /**
             * Clear our from-repository requirement, return ourself.
             *
             * \since 0.55
             */
            PartiallyMadePackageDepSpec & clear_from_repository();

            /**
             * Set our installable-to-repository requirement, return ourself.
             *
             * \since 0.32
             */
            PartiallyMadePackageDepSpec & installable_to_repository(const InstallableToRepository &);

            /**
             * Clear our installable-to-repository requirement, return ourself.
             *
             * \since 0.55
             */
            PartiallyMadePackageDepSpec & clear_installable_to_repository();

            /**
             * Set our installed-at-path requirement, return ourself.
             *
             * \since 0.32
             */
            PartiallyMadePackageDepSpec & installed_at_path(const FSPath &);

            /**
             * Clear our installed-at-path requirement, return ourself.
             *
             * \since 0.55
             */
            PartiallyMadePackageDepSpec & clear_installed_at_path();

            /**
             * Set our installable-to-path requirement, return ourself.
             *
             * \since 0.61
             */
            PartiallyMadePackageDepSpec & installable_to_path(const FSPath &, const bool include_masked);

            /**
             * Clear our installable-to-path requirement, return ourself.
             *
             * \since 0.55
             */
            PartiallyMadePackageDepSpec & clear_installable_to_path();

            /**
             * Set our package name part requirements, return ourself.
             */
            PartiallyMadePackageDepSpec & package_name_part(const PackageNamePart &);

            /**
             * Clear our package name part requirements, return ourself.
             *
             * \since 0.55
             */
            PartiallyMadePackageDepSpec & clear_package_name_part();

            /**
             * Set our category name part requirements, return ourself.
             */
            PartiallyMadePackageDepSpec & category_name_part(const CategoryNamePart &);

            /**
             * Clear our category name part requirements, return ourself.
             *
             * \since 0.55
             */
            PartiallyMadePackageDepSpec & clear_category_name_part();

            /**
             * Add a version requirement, return ourself.
             */
            PartiallyMadePackageDepSpec & version_requirement(const VersionRequirement &);

            /**
             * Clear all version requirement, return ourself.
             *
             * \since 0.55
             */
            PartiallyMadePackageDepSpec & clear_version_requirements();

            /**
             * Set our version requirements mode, return ourself.
             */
            PartiallyMadePackageDepSpec & version_requirements_mode(const VersionRequirementsMode &);

            /**
             * Add an additional requirement, return ourself.
             */
            PartiallyMadePackageDepSpec & additional_requirement(
                    const std::shared_ptr<const AdditionalPackageDepSpecRequirement> &);

            /**
             * Clear additional requirements, return ourself.
             *
             * \since 0.41
             */
            PartiallyMadePackageDepSpec & clear_additional_requirements();

            /**
             * Turn ourselves into a PackageDepSpec.
             */
            operator const PackageDepSpec() const;

            /**
             * Explicitly turn ourselves into a PackageDepSpec.
             */
            const PackageDepSpec to_package_dep_spec() const;
    };

    extern template class Pimp<PartiallyMadePackageDepSpec>;
}

#endif
