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
#include <paludis/package_dep_spec_requirement-fwd.hh>
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
             * All our requirements.
             *
             * \since 0.61
             */
            const std::shared_ptr<const PackageDepSpecRequirementSequence> requirements() const;

            /**
             * Our options.
             *
             * \since 0.61
             */
            const PackageDepSpecDataOptions options() const;

            /**
             * Fetch one of our NameRequirement requirements, if we have any, or
             * a null pointer otherwise.
             *
             * \since 0.61
             */
            const std::shared_ptr<const NameRequirement> package_name_requirement() const PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Fetch our ExactSlotRequirement, if we have one, or a null pointer otherwise.
             *
             * If we have multiple ExactSlotRequirement requirements, returns one such
             * requirement.
             *
             * \since 0.61
             */
            const std::shared_ptr<const ExactSlotRequirement> exact_slot_requirement() const;
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
             * Add a package requirement.
             *
             * \return *this
             */
            MutablePackageDepSpecData & require_package(const QualifiedPackageName &);

            /**
             * Clear any package requirements.
             *
             * \return *this
             */
            MutablePackageDepSpecData & unrequire_package();

            /**
             * Add a package name part requirement.
             *
             * \return *this
             */
            MutablePackageDepSpecData & require_package_name_part(const PackageNamePart &);

            /**
             * Clear any package name part requirements.
             *
             * \return *this
             */
            MutablePackageDepSpecData & unrequire_package_name_part();

            /**
             * Add a category name part requirement.
             *
             * \return *this
             */
            MutablePackageDepSpecData & require_category_name_part(const CategoryNamePart &);

            /**
             * Clear any category name part requirements.
             *
             * \return *this
             */
            MutablePackageDepSpecData & unrequire_category_name_part();

            /**
             * Add a version requirement.
             *
             * The combiner must be vcc_and if this is the first version
             * requirement.
             *
             * \return *this
             */
            MutablePackageDepSpecData & require_version(const VersionRequirementCombiner, const VersionOperator &, const VersionSpec &);

            /**
             * Clear any version requirements.
             *
             * \return *this
             */
            MutablePackageDepSpecData & unrequire_versions();

            /**
             * Add an exact slot requirement.
             *
             * \return *this
             */
            MutablePackageDepSpecData & require_exact_slot(const SlotName &, const bool locked);

            /**
             * Clear any exact slot requirements.
             *
             * \return *this
             */
            MutablePackageDepSpecData & unrequire_exact_slot();

            /**
             * Add an in repository requirement.
             *
             * \return *this
             */
            MutablePackageDepSpecData & require_in_repository(const RepositoryName &);

            /**
             * Clear any in repository requirement.
             *
             * \return *this
             */
            MutablePackageDepSpecData & unrequire_in_repository();

            /**
             * Add an installable to path requirement.
             *
             * \return *this
             */
            MutablePackageDepSpecData & require_installable_to_path(const FSPath &, const bool);

            /**
             * Clear any installable to path requirement.
             *
             * \return *this
             */
            MutablePackageDepSpecData & unrequire_installable_to_path();

            /**
             * Add an installable to repository requirement.
             *
             * \return *this
             */
            MutablePackageDepSpecData & require_installable_to_repository(const RepositoryName &, const bool);

            /**
             * Clear any installable to repository requirement.
             *
             * \return *this
             */
            MutablePackageDepSpecData & unrequire_installable_to_repository();

            /**
             * Add a from repository requirement.
             *
             * \return *this
             */
            MutablePackageDepSpecData & require_from_repository(const RepositoryName &);

            /**
             * Clear any from repository requirement.
             *
             * \return *this
             */
            MutablePackageDepSpecData & unrequire_from_repository();

            /**
             * Add an installed at path requirement.
             *
             * \return *this
             */
            MutablePackageDepSpecData & require_installed_at_path(const FSPath &);

            /**
             * Clear any installed at path requirement.
             *
             * \return *this
             */
            MutablePackageDepSpecData & unrequire_installed_at_path();

            /**
             * Add an any slot requirement.
             *
             * \return *this
             */
            MutablePackageDepSpecData & require_any_slot(const bool);

            /**
             * Clear our AnySlotRequirement, if we have one.
             *
             * \return *this
             */
            MutablePackageDepSpecData & unrequire_any_slot();

            /**
             * Add a choice requirement.
             *
             * \return *this
             */
            MutablePackageDepSpecData & require_choice(const std::shared_ptr<const ChoiceRequirement> &);

            /**
             * Clear any choice requirements.
             *
             * \return *this
             */
            MutablePackageDepSpecData & unrequire_choices();

            /**
             * Add a key requirement.
             *
             * \return *this
             */
            MutablePackageDepSpecData & require_key(
                    const KeyRequirementKeyType, const std::string &, const KeyRequirementOperation, const std::string &);

            /**
             * Clear any key requirements.
             *
             * \return *this
             */
            MutablePackageDepSpecData & unrequire_keys();

            /**
             * Convert ourself to a PackageDepSpec.
             */
            operator PackageDepSpec() const;
    };
}

#endif
