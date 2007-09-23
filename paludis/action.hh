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
#include <paludis/util/sr.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/sequence-fwd.hh>

namespace paludis
{

#include <paludis/action-sr.hh>

    struct ActionVisitorTypes :
        VisitorTypes<
            ActionVisitorTypes,
            Action,
            InstallAction,
            FetchAction,
            InstalledAction,
            UninstallAction,
            PretendAction,
            ConfigAction,
            InfoAction
        >
    {
    };

    struct SupportsActionTestVisitorTypes :
        VisitorTypes<
            SupportsActionTestVisitorTypes,
            SupportsActionTestBase,
            SupportsActionTest<InstallAction>,
            SupportsActionTest<FetchAction>,
            SupportsActionTest<InstalledAction>,
            SupportsActionTest<UninstallAction>,
            SupportsActionTest<PretendAction>,
            SupportsActionTest<ConfigAction>,
            SupportsActionTest<InfoAction>
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

    class PALUDIS_VISIBLE FetchAction :
        public Action,
        private PrivateImplementationPattern<FetchAction>,
        public MutableAcceptInterfaceVisitsThis<ActionVisitorTypes, FetchAction>
    {
        public:
            FetchAction(const FetchActionOptions &);
            ~FetchAction();

            const FetchActionOptions & options;
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

    class PALUDIS_VISIBLE InfoAction:
        public Action,
        public MutableAcceptInterfaceVisitsThis<ActionVisitorTypes, InfoAction>
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

    std::ostream & operator<< (std::ostream &, const Action &) PALUDIS_VISIBLE;

    /**
     * Parent class for action errors.
     *
     * \ingroup grpexceptions
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE ActionError :
        public Exception
    {
        public:
            /**
             * Constructor.
             */
            ActionError(const std::string & msg) throw ();
    };

    class PALUDIS_VISIBLE UnsupportedActionError :
        public ActionError
    {
        public:
            UnsupportedActionError(const PackageID &, const Action &) throw ();
    };

    /**
     * Thrown if an install fails.
     *
     * \ingroup grpexceptions
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE InstallActionError : public ActionError
    {
        public:
            /**
             * Constructor.
             */
            InstallActionError(const std::string & msg) throw ();
    };

    /**
     * Thrown if a fetch fails.
     *
     * \ingroup grpexceptions
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE FetchActionError :
        public ActionError
    {
        private:
            const tr1::shared_ptr<const Sequence<FetchActionFailure> > _failures;

        public:
            FetchActionError(const std::string &) throw ();
            FetchActionError(const std::string &, const tr1::shared_ptr<const Sequence<FetchActionFailure> > &) throw ();

            const tr1::shared_ptr<const Sequence<FetchActionFailure> > failures() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    /**
     * Thrown if an uninstall fails.
     *
     * \ingroup grpexceptions
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE UninstallActionError : public ActionError
    {
        public:
            /**
             * Constructor.
             */
            UninstallActionError(const std::string & msg) throw ();
    };

    /**
     * Thrown if a configure fails.
     *
     * \ingroup grpexceptions
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE ConfigActionError : public ActionError
    {
        public:
            /**
             * Constructor.
             */
            ConfigActionError(const std::string & msg) throw ();
    };

    /**
     * Thrown if an info fails.
     *
     * \ingroup grpexceptions
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE InfoActionError : public ActionError
    {
        public:
            /**
             * Constructor.
             */
            InfoActionError(const std::string & msg) throw ();
    };
}

#endif
