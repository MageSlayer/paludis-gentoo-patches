/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_HANDLED_INFORMATION_HH
#define PALUDIS_GUARD_PALUDIS_HANDLED_INFORMATION_HH 1

#include <paludis/handled_information-fwd.hh>
#include <paludis/util/visitor.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/package_id-fwd.hh>

/** \file
 * Declarations for DepListEntryHandled classes, which are used by DepList and
 * InstallTask to keep track of whether a DepListEntry has been handled yet.
 *
 * \ingroup g_dep_list
 *
 * \section Examples
 *
 * - None at this time.
 */

namespace paludis
{
    /**
     * Types for a visitor that can visit a DepListEntry subclass.
     *
     * \ingroup g_dep_list
     * \since 0.26
     * \nosubgrouping
     */
    struct DepListEntryHandledVisitorTypes :
        VisitorTypes<
            DepListEntryHandledVisitorTypes,
            DepListEntryHandled,
            DepListEntryHandledSuccess,
            DepListEntryHandledSkippedUnsatisfied,
            DepListEntryHandledSkippedDependent,
            DepListEntryHandledFailed,
            DepListEntryUnhandled,
            DepListEntryNoHandlingRequired
        >
    {
    };

    /**
     * Represents a DepListEntry that has been handled.
     *
     * \ingroup g_dep_list
     * \since 0.26
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE DepListEntryHandled :
        public virtual ConstAcceptInterface<DepListEntryHandledVisitorTypes>
    {
        public:
            virtual ~DepListEntryHandled() = 0;
    };

    /**
     * Represents a DepListEntry that has not been handled.
     *
     * \ingroup g_dep_list
     * \since 0.26
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE DepListEntryUnhandled :
        public DepListEntryHandled,
        public ConstAcceptInterfaceVisitsThis<DepListEntryHandledVisitorTypes, DepListEntryUnhandled>
    {
    };

    /**
     * Represents a DepListEntry that requires no handling.
     *
     * \ingroup g_dep_list
     * \since 0.26
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE DepListEntryNoHandlingRequired :
        public DepListEntryHandled,
        public ConstAcceptInterfaceVisitsThis<DepListEntryHandledVisitorTypes, DepListEntryNoHandlingRequired>
    {
    };

    /**
     * Represents a DepListEntry that has been handled successfully.
     *
     * \ingroup g_dep_list
     * \since 0.26
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE DepListEntryHandledSuccess :
        public DepListEntryHandled,
        public ConstAcceptInterfaceVisitsThis<DepListEntryHandledVisitorTypes, DepListEntryHandledSuccess>
    {
    };

    /**
     * Represents a DepListEntry that was skipped because of unsatisfied
     * dependencies.
     *
     * \ingroup g_dep_list
     * \since 0.26
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE DepListEntryHandledSkippedUnsatisfied :
        public DepListEntryHandled,
        public ConstAcceptInterfaceVisitsThis<DepListEntryHandledVisitorTypes, DepListEntryHandledSkippedUnsatisfied>,
        private PrivateImplementationPattern<DepListEntryHandledSkippedUnsatisfied>
    {
        public:
            ///\name Basic operations
            ///\{

            DepListEntryHandledSkippedUnsatisfied(const PackageDepSpec &);
            ~DepListEntryHandledSkippedUnsatisfied();

            ///\}

            /**
             * What PackageDepSpec was unsatisfied? If multiple specs were
             * unsatisfied, returns one of them.
             */
            const PackageDepSpec spec() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    /**
     * Represents a DepListEntry that was skipped because of a dependency upon a
     * failed package.
     *
     * \ingroup g_dep_list
     * \since 0.26
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE DepListEntryHandledSkippedDependent :
        public DepListEntryHandled,
        public ConstAcceptInterfaceVisitsThis<DepListEntryHandledVisitorTypes, DepListEntryHandledSkippedDependent>,
        private PrivateImplementationPattern<DepListEntryHandledSkippedDependent>
    {
        public:
            ///\name Basic operations
            ///\{

            DepListEntryHandledSkippedDependent(const tr1::shared_ptr<const PackageID> &);
            ~DepListEntryHandledSkippedDependent();

            ///\}

            /**
             * Upon which PackageID are we dependent? If multiple dependent IDs are
             * unsatisfied, returns one of them.
             */
            const tr1::shared_ptr<const PackageID> id() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    /**
     * Represents a DepListEntry that failed.
     *
     * \ingroup g_dep_list
     * \since 0.26
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE DepListEntryHandledFailed :
        public DepListEntryHandled,
        public ConstAcceptInterfaceVisitsThis<DepListEntryHandledVisitorTypes, DepListEntryHandledFailed>
    {
    };
}

#endif
