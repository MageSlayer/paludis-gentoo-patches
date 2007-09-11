/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_DEP_LIST_HANDLED_INFORMATION_HH
#define PALUDIS_GUARD_PALUDIS_DEP_LIST_HANDLED_INFORMATION_HH 1

#include <paludis/dep_list/handled_information-fwd.hh>
#include <paludis/util/visitor.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/dep_spec-fwd.hh>

namespace paludis
{
    struct DepListEntryHandledVisitorTypes :
        VisitorTypes<
            DepListEntryHandledVisitorTypes,
            DepListEntryHandled,
            DepListEntryHandledSuccess,
            DepListEntryHandledSkippedUnsatisfied,
            DepListEntryHandledFailed,
            DepListEntryUnhandled,
            DepListEntryNoHandlingRequired
        >
    {
    };

    class PALUDIS_VISIBLE DepListEntryHandled :
        public virtual ConstAcceptInterface<DepListEntryHandledVisitorTypes>
    {
        public:
            virtual ~DepListEntryHandled() = 0;
    };

    class PALUDIS_VISIBLE DepListEntryUnhandled :
        public DepListEntryHandled,
        public ConstAcceptInterfaceVisitsThis<DepListEntryHandledVisitorTypes, DepListEntryUnhandled>
    {
    };

    class PALUDIS_VISIBLE DepListEntryNoHandlingRequired :
        public DepListEntryHandled,
        public ConstAcceptInterfaceVisitsThis<DepListEntryHandledVisitorTypes, DepListEntryNoHandlingRequired>
    {
    };

    class PALUDIS_VISIBLE DepListEntryHandledSuccess :
        public DepListEntryHandled,
        public ConstAcceptInterfaceVisitsThis<DepListEntryHandledVisitorTypes, DepListEntryHandledSuccess>
    {
    };

    class PALUDIS_VISIBLE DepListEntryHandledSkippedUnsatisfied :
        public DepListEntryHandled,
        public ConstAcceptInterfaceVisitsThis<DepListEntryHandledVisitorTypes, DepListEntryHandledSkippedUnsatisfied>,
        private PrivateImplementationPattern<DepListEntryHandledSkippedUnsatisfied>
    {
        public:
            DepListEntryHandledSkippedUnsatisfied(const PackageDepSpec &);
            ~DepListEntryHandledSkippedUnsatisfied();

            const PackageDepSpec spec() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE DepListEntryHandledFailed :
        public DepListEntryHandled,
        public ConstAcceptInterfaceVisitsThis<DepListEntryHandledVisitorTypes, DepListEntryHandledFailed>
    {
    };
}

#endif
