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

#ifndef PALUDIS_GUARD_PALUDIS_USER_DEP_SPEC_HH
#define PALUDIS_GUARD_PALUDIS_USER_DEP_SPEC_HH 1

#include <paludis/user_dep_spec-fwd.hh>
#include <paludis/dep_spec.hh>
#include <paludis/slot_requirement.hh>
#include <paludis/filter.hh>
#include <paludis/additional_package_dep_spec_requirement.hh>
#include <paludis/util/pimp.hh>

namespace paludis
{
    /**
     * Create a PackageDepSpec from user input.
     *
     * \ingroup g_dep_spec
     * \since 0.28
     */
    PackageDepSpec parse_user_package_dep_spec(
            const std::string &,
            const Environment * const,
            const UserPackageDepSpecOptions &,
            const Filter & = filter::All()) PALUDIS_VISIBLE;

    /**
     * Create a PackageDepSpec from user input, restricted to not having
     * an Environment available.
     *
     * For use in test cases; should not be used elsewhere.
     *
     * \ingroup g_dep_spec
     * \since 0.61
     */
    PackageDepSpec envless_parse_package_dep_spec_for_tests(
            const std::string &) PALUDIS_VISIBLE;

    class PALUDIS_VISIBLE UserSlotExactRequirement :
        public SlotExactRequirement
    {
        private:
            const SlotName _s;

        public:
            ///\name Basic operations
            ///\{

            UserSlotExactRequirement(const SlotName &);

            ///\}

            virtual const SlotName slot() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual const std::string as_string() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual bool from_any_locked() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    /**
     * A key requirement for a user PackageDepSpec.
     *
     * \since 0.36
     * \ingroup g_dep_spec
     */
    class PALUDIS_VISIBLE UserKeyRequirement :
        public AdditionalPackageDepSpecRequirement
    {
        private:
            Pimp<UserKeyRequirement> _imp;

        public:
            ///\name Basic operations
            ///\{

            UserKeyRequirement(const std::string &);
            ~UserKeyRequirement();

            ///\}

            virtual const std::pair<bool, std::string> requirement_met(
                    const Environment * const, const ChangedChoices *,
                    const std::shared_ptr<const PackageID> &,
                    const std::shared_ptr<const PackageID> &,
                    const ChangedChoices * const) const PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual const std::string as_human_string(
                    const std::shared_ptr<const PackageID> &) const PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual const std::string as_raw_string() const PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual Tribool accumulate_changes_to_make_met(
                    const Environment * const,
                    const ChangedChoices * const,
                    const std::shared_ptr<const PackageID> &,
                    const std::shared_ptr<const PackageID> &,
                    ChangedChoices &) const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    extern template class Pimp<UserKeyRequirement>;
}

#endif
