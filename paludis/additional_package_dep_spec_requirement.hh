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

#ifndef PALUDIS_GUARD_PALUDIS_ADDITIONAL_PACKAGE_DEP_SPEC_REQUIREMENT_HH
#define PALUDIS_GUARD_PALUDIS_ADDITIONAL_PACKAGE_DEP_SPEC_REQUIREMENT_HH 1

#include <paludis/additional_package_dep_spec_requirement-fwd.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/changed_choices-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <utility>

namespace paludis
{
    /**
     * An additional requirement for a PackageDepSpec.
     *
     * \since 0.26
     * \ingroup g_dep_spec
     */
    class PALUDIS_VISIBLE AdditionalPackageDepSpecRequirement
    {
        public:
            AdditionalPackageDepSpecRequirement() = default;
            virtual ~AdditionalPackageDepSpecRequirement();

            AdditionalPackageDepSpecRequirement(const AdditionalPackageDepSpecRequirement &) = delete;
            AdditionalPackageDepSpecRequirement & operator= (const AdditionalPackageDepSpecRequirement &) = delete;

            /**
             * Is our requirement met for a given PackageID?
             *
             * The string in the return type might be a description of why the
             * requirement was not met. Sometimes better messages can be given
             * than simply the return value of as_human_string() when the ID to
             * be matched is known. If the bool is false, the string is
             * meaningless.
             *
             * \since 0.44 returns pair<bool, std::string>
             * \since 0.51 takes optional ChangedChoices arguments
             */
            virtual const std::pair<bool, std::string> requirement_met(
                    const Environment * const,
                    const ChangedChoices * const maybe_changes_to_owner,
                    const PackageID &,
                    const ChangedChoices * const maybe_changes_to_target) const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * If possible, indicate which choices to change to make our
             * requirement met for a particular ID.
             *
             * Verifies that the ID has the appropriate choice, and that that
             * choice isn't locked.
             *
             * \since 0.51
             */
            virtual bool accumulate_changes_to_make_met(
                    const Environment * const,
                    const ChangedChoices * const maybe_changes_to_owner,
                    const std::shared_ptr<const PackageID> &,
                    ChangedChoices &) const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Return a human readable string representation of ourself.
             */
            virtual const std::string as_human_string() const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Return a raw string representation of ourself.
             */
            virtual const std::string as_raw_string() const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    extern template class Sequence<std::shared_ptr<const AdditionalPackageDepSpecRequirement> >;
    extern template class WrappedForwardIterator<AdditionalPackageDepSpecRequirements::ConstIteratorTag,
           const std::shared_ptr<const AdditionalPackageDepSpecRequirement> >;
}

#endif
