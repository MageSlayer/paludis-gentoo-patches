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

#ifndef PALUDIS_GUARD_PALUDIS_ACTION_HH
#define PALUDIS_GUARD_PALUDIS_ACTION_HH 1

#include <paludis/action-fwd.hh>
#include <paludis/repository-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/visitor.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/private_implementation_pattern.hh>

namespace paludis
{

#include <paludis/action-sr.hh>

    struct ActionVisitorTypes :
        VisitorTypes<
            ActionVisitorTypes,
            Action,
            InstallAction,
            InstalledAction,
            UninstallAction,
            PretendAction,
            ConfigAction
        >
    {
    };

    struct SupportsActionTestVisitorTypes :
        VisitorTypes<
            SupportsActionTestVisitorTypes,
            SupportsActionTestBase,
            SupportsActionTest<InstallAction>,
            SupportsActionTest<InstalledAction>,
            SupportsActionTest<UninstallAction>,
            SupportsActionTest<PretendAction>,
            SupportsActionTest<ConfigAction>
        >
    {
    };

    class PALUDIS_VISIBLE Action :
        public virtual MutableAcceptInterface<ActionVisitorTypes>
    {
        public:
            virtual ~Action() = 0;
    };

    class PALUDIS_VISIBLE InstallAction :
        public Action,
        private PrivateImplementationPattern<InstallAction>,
        public MutableAcceptInterfaceVisitsThis<ActionVisitorTypes, InstallAction>
    {
        public:
            InstallAction(const InstallActionOptions &);
            ~InstallAction();

            const InstallActionOptions & options;
    };

    class PALUDIS_VISIBLE UninstallAction :
        public Action,
        private PrivateImplementationPattern<UninstallAction>,
        public MutableAcceptInterfaceVisitsThis<ActionVisitorTypes, UninstallAction>
    {
        public:
            UninstallAction(const UninstallActionOptions &);
            ~UninstallAction();

            const UninstallActionOptions & options;
    };

    class PALUDIS_VISIBLE InstalledAction :
        public Action,
        public MutableAcceptInterfaceVisitsThis<ActionVisitorTypes, InstalledAction>
    {
    };

    class PALUDIS_VISIBLE PretendAction :
        public Action,
        private PrivateImplementationPattern<PretendAction>,
        public MutableAcceptInterfaceVisitsThis<ActionVisitorTypes, PretendAction>
    {
        public:
            PretendAction();
            ~PretendAction();

            const bool failed() const PALUDIS_ATTRIBUTE((warn_unused_result));
            void set_failed();
    };

    class PALUDIS_VISIBLE ConfigAction :
        public Action,
        public MutableAcceptInterfaceVisitsThis<ActionVisitorTypes, ConfigAction>
    {
    };

    class PALUDIS_VISIBLE SupportsActionTestBase :
        public virtual MutableAcceptInterface<SupportsActionTestVisitorTypes>
    {
        public:
            virtual ~SupportsActionTestBase() = 0;
    };

    template <typename A_>
    class PALUDIS_VISIBLE SupportsActionTest :
        public SupportsActionTestBase,
        public MutableAcceptInterfaceVisitsThis<SupportsActionTestVisitorTypes, SupportsActionTest<A_> >
    {
    };

    class PALUDIS_VISIBLE UnsupportedActionError :
        public Exception
    {
        public:
            UnsupportedActionError(const PackageID &, const Action &) throw ();
    };

    std::ostream & operator<< (std::ostream &, const Action &) PALUDIS_VISIBLE;
}

#endif
