/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010 Ciaran McCreesh
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
    class PALUDIS_VISIBLE PartiallyMadePackageDepSpec :
        private Pimp<PartiallyMadePackageDepSpec>
    {
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
             * Set our slot requirements, return ourself.
             */
            PartiallyMadePackageDepSpec & slot_requirement(const std::shared_ptr<const SlotRequirement> &);

            /**
             * Set our in-repository requirement, return ourself.
             */
            PartiallyMadePackageDepSpec & in_repository(const RepositoryName &);

            /**
             * Set our from-repository requirement, return ourself.
             */
            PartiallyMadePackageDepSpec & from_repository(const RepositoryName &);

            /**
             * Set our installable-to-repository requirement, return ourself.
             *
             * \since 0.32
             */
            PartiallyMadePackageDepSpec & installable_to_repository(const InstallableToRepository &);

            /**
             * Set our installed-at-path requirement, return ourself.
             *
             * \since 0.32
             */
            PartiallyMadePackageDepSpec & installed_at_path(const FSPath &);

            /**
             * Set our installable-to-path requirement, return ourself.
             *
             * \since 0.32
             */
            PartiallyMadePackageDepSpec & installable_to_path(const InstallableToPath &);

            /**
             * Set our package name part requirements, return ourself.
             */
            PartiallyMadePackageDepSpec & package_name_part(const PackageNamePart &);

            /**
             * Set our category name part requirements, return ourself.
             */
            PartiallyMadePackageDepSpec & category_name_part(const CategoryNamePart &);

            /**
             * Add a version requirement, return ourself.
             */
            PartiallyMadePackageDepSpec & version_requirement(const VersionRequirement &);

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
             * Add annotations
             */
            PartiallyMadePackageDepSpec & annotations(
                    const std::shared_ptr<const MetadataSectionKey> &);

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
