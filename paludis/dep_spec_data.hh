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

#ifndef PALUDIS_GUARD_PALUDIS_DEP_SPEC_DATA_HH
#define PALUDIS_GUARD_PALUDIS_DEP_SPEC_DATA_HH 1

#include <paludis/dep_spec_data-fwd.hh>
#include <paludis/changed_choices-fwd.hh>
#include <paludis/name-fwd.hh>
#include <paludis/version_operator-fwd.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/package_dep_spec_constraint-fwd.hh>
#include <paludis/version_spec-fwd.hh>

#include <paludis/util/attributes.hh>
#include <paludis/util/fs_path-fwd.hh>
#include <paludis/util/pimp.hh>

#include <string>
#include <memory>

namespace paludis
{
    /**
     * Data for a ConditionalDepSpec.
     *
     * \since 0.26
     * \ingroup g_dep_spec
     */
    class PALUDIS_VISIBLE ConditionalDepSpecData
    {
        public:
            ///\name Basic operations
            ///\{

            virtual ~ConditionalDepSpecData();

            ///\}

            /**
             * Fetch ourself as a string.
             */
            virtual std::string as_string() const = 0;

            /**
             * Fetch the result for condition_met.
             *
             * \since 0.58 takes env, package_id
             */
            virtual bool condition_met(
                    const Environment * const,
                    const std::shared_ptr<const PackageID> &) const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Fetch the result for condition_would_be_met_when.
             *
             * \since 0.58 takes env, package_id
             */
            virtual bool condition_would_be_met_when(
                    const Environment * const,
                    const std::shared_ptr<const PackageID> &,
                    const ChangedChoices &) const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Fetch the result for condition_meetable.
             *
             * \since 0.58 takes env, package_id
             */
            virtual bool condition_meetable(
                    const Environment * const,
                    const std::shared_ptr<const PackageID> &) const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    /**
     * Data for a PackageDepSpec.
     *
     * \since 0.26
     * \since 0.61 is not abstract
     * \ingroup g_dep_spec
     */
    class PALUDIS_VISIBLE PackageDepSpecData
    {
        protected:
            Pimp<PackageDepSpecData> _imp;

            PackageDepSpecData(const PackageDepSpecData &);

        public:
            ///\name Basic operations
            ///\{

            explicit PackageDepSpecData(const PackageDepSpecDataOptions &);

            virtual ~PackageDepSpecData();

            ///\}

            /**
             * Fetch ourself as a string.
             */
            std::string as_string() const;

            /**
             * Fetch the single NameConstraint, if we have one, or
             * a null pointer otherwise.
             *
             * \since 0.61
             */
            const std::shared_ptr<const NameConstraint> package_name_constraint() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Fetch the single PackageNamePartConstraint, if we have one, or
             * a null pointer otherwise.
             *
             * \since 0.61
             */
            const std::shared_ptr<const PackageNamePartConstraint> package_name_part_constraint() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Fetch the single CategoryNamePartConstraint, if we have one, or
             * a null pointer otherwise.
             *
             * \since 0.61
             */
            const std::shared_ptr<const CategoryNamePartConstraint> category_name_part_constraint() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Fetch all our VersionConstraints, if we have any, or
             * a null pointer otherwise.
             *
             * \since 0.61
             */
            const std::shared_ptr<const VersionConstraintSequence> all_version_constraints() const;

            /**
             * Fetch the single ExactSlotConstraint, if we have one, or
             * a null pointer otherwise.
             *
             * \since 0.61
             */
            const std::shared_ptr<const ExactSlotConstraint> exact_slot_constraint() const;

            /**
             * Fetch the single AnySlotConstraint, if we have one, or
             * a null pointer otherwise.
             *
             * \since 0.61
             */
            const std::shared_ptr<const AnySlotConstraint> any_slot_constraint() const;

            /**
             * Fetch the single InRepositoryConstraint, if we have one, or
             * a null pointer otherwise.
             *
             * \since 0.61
             */
            const std::shared_ptr<const InRepositoryConstraint> in_repository_constraint() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Fetch the single InstallableToRepositoryConstraint, if we have one, or
             *
             * \since 0.61
             */
            const std::shared_ptr<const InstallableToRepositoryConstraint> installable_to_repository_constraint() const;

            /**
             * Fetch the single FromRepositoryConstraint, if we have one, or
             * a null pointer otherwise.
             *
             * \since 0.61
             */
            const std::shared_ptr<const FromRepositoryConstraint> from_repository_constraint() const;

            /**
             * Fetch the single InstalledAtPathConstraint, if we have one, or
             * a null pointer otherwise.
             *
             * \since 0.61
             */
            const std::shared_ptr<const InstalledAtPathConstraint> installed_at_path_constraint() const;

            /**
             * Fetch the single InstallableToPathConstraint, if we have one, or
             * a null pointer otherwise.
             *
             * \since 0.61
             */
            const std::shared_ptr<const InstallableToPathConstraint> installable_to_path_constraint() const;

            /**
             * Fetch all our KeyConstraints, if we have any, or
             * a null pointer otherwise.
             *
             * \since 0.61
             */
            const std::shared_ptr<const KeyConstraintSequence> all_key_constraints() const;

            /**
             * Fetch all our ChoiceConstraints, if we have any, or
             * a null pointer otherwise.
             *
             * \since 0.61
             */
            const std::shared_ptr<const ChoiceConstraintSequence> all_choice_constraints() const;

            /**
             * Our options.
             *
             * \since 0.61
             */
            const PackageDepSpecDataOptions options() const;
    };

    /**
     * Partially constructed PackageDepSpecData, which can be used to build up a
     * PackageDepSpec.
     *
     * \since 0.61
     * \ingroup g_dep_spec
     */
    class PALUDIS_VISIBLE MutablePackageDepSpecData :
        private PackageDepSpecData
    {
        public:
            explicit MutablePackageDepSpecData(const PackageDepSpecDataOptions &);

            MutablePackageDepSpecData(const PackageDepSpecData &);

            MutablePackageDepSpecData(const MutablePackageDepSpecData &);

            ~MutablePackageDepSpecData();

            /**
             * Add a package constraint.
             *
             * \return *this
             */
            MutablePackageDepSpecData & constrain_package(const QualifiedPackageName &);

            /**
             * Clear any package constraints.
             *
             * \return *this
             */
            MutablePackageDepSpecData & unconstrain_package();

            /**
             * Add a package name part constraint.
             *
             * \return *this
             */
            MutablePackageDepSpecData & constrain_package_name_part(const PackageNamePart &);

            /**
             * Clear any package name part constraints.
             *
             * \return *this
             */
            MutablePackageDepSpecData & unconstrain_package_name_part();

            /**
             * Add a category name part constraint.
             *
             * \return *this
             */
            MutablePackageDepSpecData & constrain_category_name_part(const CategoryNamePart &);

            /**
             * Clear any category name part constraints.
             *
             * \return *this
             */
            MutablePackageDepSpecData & unconstrain_category_name_part();

            /**
             * Add a version constraint.
             *
             * The combiner must be vcc_and if this is the first version
             * constraint.
             *
             * \return *this
             */
            MutablePackageDepSpecData & constrain_version(const VersionConstraintCombiner, const VersionOperator &, const VersionSpec &);

            /**
             * Clear any version constraints.
             *
             * \return *this
             */
            MutablePackageDepSpecData & unconstrain_versions();

            /**
             * Add an exact slot constraint.
             *
             * \return *this
             */
            MutablePackageDepSpecData & constrain_exact_slot(const SlotName &, const bool locked);

            /**
             * Clear any exact slot constraints.
             *
             * \return *this
             */
            MutablePackageDepSpecData & unconstrain_exact_slot();

            /**
             * Add an in repository constraint.
             *
             * \return *this
             */
            MutablePackageDepSpecData & constrain_in_repository(const RepositoryName &);

            /**
             * Clear any in repository constraint.
             *
             * \return *this
             */
            MutablePackageDepSpecData & unconstrain_in_repository();

            /**
             * Add an installable to path constraint.
             *
             * \return *this
             */
            MutablePackageDepSpecData & constrain_installable_to_path(const FSPath &, const bool);

            /**
             * Clear any installable to path constraint.
             *
             * \return *this
             */
            MutablePackageDepSpecData & unconstrain_installable_to_path();

            /**
             * Add an installable to repository constraint.
             *
             * \return *this
             */
            MutablePackageDepSpecData & constrain_installable_to_repository(const RepositoryName &, const bool);

            /**
             * Clear any installable to repository constraint.
             *
             * \return *this
             */
            MutablePackageDepSpecData & unconstrain_installable_to_repository();

            /**
             * Add a from repository constraint.
             *
             * \return *this
             */
            MutablePackageDepSpecData & constrain_from_repository(const RepositoryName &);

            /**
             * Clear any from repository constraint.
             *
             * \return *this
             */
            MutablePackageDepSpecData & unconstrain_from_repository();

            /**
             * Add an installed at path constraint.
             *
             * \return *this
             */
            MutablePackageDepSpecData & constrain_installed_at_path(const FSPath &);

            /**
             * Clear any installed at path constraint.
             *
             * \return *this
             */
            MutablePackageDepSpecData & unconstrain_installed_at_path();

            /**
             * Add an any slot constraint.
             *
             * \return *this
             */
            MutablePackageDepSpecData & constrain_any_slot(const bool);

            /**
             * Clear our AnySlotConstraint, if we have one.
             *
             * \return *this
             */
            MutablePackageDepSpecData & unconstrain_any_slot();

            /**
             * Add a choice constraint.
             *
             * \return *this
             */
            MutablePackageDepSpecData & constrain_choice(const std::shared_ptr<const ChoiceConstraint> &);

            /**
             * Clear any choice constraints.
             *
             * \return *this
             */
            MutablePackageDepSpecData & unconstrain_choices();

            /**
             * Add a key constraint.
             *
             * \return *this
             */
            MutablePackageDepSpecData & constrain_key(
                    const KeyConstraintKeyType, const std::string &, const KeyConstraintOperation, const std::string &);

            /**
             * Clear any key constraints.
             *
             * \return *this
             */
            MutablePackageDepSpecData & unconstrain_keys();

            /**
             * Convert ourself to a PackageDepSpec.
             */
            operator PackageDepSpec() const;
    };
}

#endif
