/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008 Ciaran McCreesh
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
#include <paludis/util/sequence-fwd.hh>

/** \file
 * Declarations for action-related classes.
 *
 * \ingroup g_actions
 *
 * \section Examples
 *
 * - \ref example_action.cc "example_action.cc"
 */

namespace paludis
{
    /**
     * Types for visiting an action.
     *
     * \since 0.26
     * \ingroup g_actions
     * \nosubgrouping
     */
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
            InfoAction,
            PretendFetchAction
        >
    {
    };

    /**
     * Types for visiting a supports action query.
     *
     * \since 0.26
     * \ingroup g_actions
     * \nosubgrouping
     */
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
            SupportsActionTest<InfoAction>,
            SupportsActionTest<PretendFetchAction>
        >
    {
    };

    /**
     * An Action represents an action that can be executed by a PackageID via
     * PackageID::perform_action.
     *
     * \since 0.26
     * \ingroup g_actions
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE Action :
        public virtual AcceptInterface<ActionVisitorTypes>
    {
        public:
            ///\name Basic operations
            ///\{

            virtual ~Action() = 0;

            ///\}
    };

    /**
     * An InstallAction is used by InstallTask to perform a build / install on a
     * PackageID.
     *
     * \since 0.26
     * \ingroup g_actions
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE InstallAction :
        public Action,
        private PrivateImplementationPattern<InstallAction>,
        public AcceptInterfaceVisitsThis<ActionVisitorTypes, InstallAction>
    {
        public:
            ///\name Basic operations
            ///\{

            InstallAction(const InstallActionOptions &);
            ~InstallAction();

            ///\}

            /// Options for the action.
            const InstallActionOptions & options;
    };

    /**
     * A FetchAction can be used to fetch source files for a PackageID using
     * PackageID::perform_action.
     *
     * \since 0.26
     * \ingroup g_actions
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE FetchAction :
        public Action,
        private PrivateImplementationPattern<FetchAction>,
        public AcceptInterfaceVisitsThis<ActionVisitorTypes, FetchAction>
    {
        public:
            ///\name Basic operations
            ///\{

            FetchAction(const FetchActionOptions &);
            ~FetchAction();

            ///\}

            /// Options for the action.
            const FetchActionOptions & options;
    };

    /**
     * An UninstallAction is used by UninstallTask to uninstall a PackageID.
     *
     * \since 0.26
     * \ingroup g_actions
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE UninstallAction :
        public Action,
        private PrivateImplementationPattern<UninstallAction>,
        public AcceptInterfaceVisitsThis<ActionVisitorTypes, UninstallAction>
    {
        public:
            ///\name Basic operations
            ///\{

            UninstallAction(const UninstallActionOptions &);
            ~UninstallAction();

            ///\}

            /// Options for the action.
            const UninstallActionOptions & options;
    };

    /**
     * InstalledAction is a dummy action used by SupportsActionTest and
     * query::SupportsAction to determine whether a PackageID is installed.
     *
     * Performing an InstalledAction does not make sense and will do nothing.
     *
     * \since 0.26
     * \ingroup g_actions
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE InstalledAction :
        public Action,
        public AcceptInterfaceVisitsThis<ActionVisitorTypes, InstalledAction>
    {
    };

    /**
     * A PretendAction is used by InstallTask to handle install-pretend-phase
     * checks on a PackageID.
     *
     * \since 0.26
     * \ingroup g_actions
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE PretendAction :
        public Action,
        private PrivateImplementationPattern<PretendAction>,
        public AcceptInterfaceVisitsThis<ActionVisitorTypes, PretendAction>
    {
        public:
            ///\name Basic operations
            ///\{

            PretendAction();
            ~PretendAction();

            ///\}

            /// Did our pretend phase fail?
            bool failed() const PALUDIS_ATTRIBUTE((warn_unused_result));

            /// Mark the action as failed.
            void set_failed();
    };

    /**
     * A PretendFetchAction is used to get information about a fetch that will take
     * place on a PackageID.
     *
     * \since 0.26
     * \ingroup g_actions
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE PretendFetchAction :
        public Action,
        private PrivateImplementationPattern<PretendFetchAction>,
        public AcceptInterfaceVisitsThis<ActionVisitorTypes, PretendFetchAction>
    {
        public:
            ///\name Basic operations
            ///\{

            PretendFetchAction(const FetchActionOptions &);
            ~PretendFetchAction();

            ///\}

            /// Options for the FetchAction we will use.
            const FetchActionOptions & options;

            /// Signal that we will fetch a particular file.
            virtual void will_fetch(const FSEntry & destination, const unsigned long size_in_bytes) = 0;
    };

    /**
     * A ConfigAction is used via PackageID::perform_action to execute
     * post-install configuration (for example, via 'paludis --config')
     * on a PackageID.
     *
     * \since 0.26
     * \ingroup g_actions
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE ConfigAction :
        public Action,
        public AcceptInterfaceVisitsThis<ActionVisitorTypes, ConfigAction>
    {
    };

    /**
     * An InfoAction is used via PackageID::perform_action to execute
     * additional information (for example, via 'paludis --info')
     * on a PackageID.
     *
     * This action potentially makes sense for both installed and
     * installable packages. Unlike Ebuild EAPI-0 'pkg_info', this
     * action is not specifically tied to installed packages.
     *
     * \since 0.26
     * \ingroup g_actions
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE InfoAction:
        public Action,
        public AcceptInterfaceVisitsThis<ActionVisitorTypes, InfoAction>
    {
    };

    /**
     * Base class for SupportsActionTest<>.
     *
     * \see SupportsActionTest<>
     * \since 0.26
     * \ingroup g_actions
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE SupportsActionTestBase :
        public virtual AcceptInterface<SupportsActionTestVisitorTypes>
    {
        public:
            virtual ~SupportsActionTestBase() = 0;
    };

    /**
     * Instantiated with an Action subclass as its template parameter,
     * SupportsActionTest<> is used by PackageID::supports_action and
     * Repository::some_ids_might_support_action to query whether a
     * particular action is supported by that PackageID or potentially
     * supported by some IDs in that Repository.
     *
     * Use of a separate class, rather than a mere Action, avoids the
     * need to create bogus options for the more complicated Action
     * subclasses.
     *
     * \since 0.26
     * \ingroup g_actions
     * \nosubgrouping
     */
    template <typename A_>
    class PALUDIS_VISIBLE SupportsActionTest :
        public SupportsActionTestBase,
        public AcceptInterfaceVisitsThis<SupportsActionTestVisitorTypes, SupportsActionTest<A_> >
    {
    };

    /**
     * An Action can be written to a std::ostream.
     *
     * \since 0.26
     * \ingroup g_actions
     * \nosubgrouping
     */
    std::ostream & operator<< (std::ostream &, const Action &) PALUDIS_VISIBLE;

    /**
     * Parent class for Action related errors.
     *
     * \ingroup g_actions
     * \ingroup g_exceptions
     * \since 0.26
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE ActionError :
        public Exception
    {
        public:
            ///\name Basic operations
            ///\{

            ActionError(const std::string & msg) throw ();

            ///\}
    };

    /**
     * Thrown if a PackageID is asked to perform an Action that it does
     * not support.
     *
     * \ingroup g_exceptions
     * \ingroup g_actions
     * \since 0.26
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE UnsupportedActionError :
        public ActionError
    {
        public:
            ///\name Basic operations
            ///\{

            UnsupportedActionError(const PackageID &, const Action &) throw ();

            ///\}
    };

    /**
     * Thrown if a PackageID fails to perform an InstallAction.
     *
     * \ingroup g_exceptions
     * \ingroup g_actions
     * \since 0.26
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE InstallActionError : public ActionError
    {
        public:
            ///\name Basic operations
            ///\{

            InstallActionError(const std::string & msg) throw ();

            ///\}
    };

    /**
     * Thrown if a PackageID fails to perform a FetchAction.
     *
     * \ingroup g_exceptions
     * \ingroup g_actions
     * \since 0.26
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE FetchActionError :
        public ActionError
    {
        private:
            const std::tr1::shared_ptr<const Sequence<FetchActionFailure> > _failures;

        public:
            ///\name Basic operations
            ///\{

            FetchActionError(const std::string &) throw ();
            FetchActionError(const std::string &, const std::tr1::shared_ptr<const Sequence<FetchActionFailure> > &) throw ();
            ~FetchActionError() throw ();

            ///\}

            /// More information about failed fetches.
            const std::tr1::shared_ptr<const Sequence<FetchActionFailure> > failures() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    /**
     * Thrown if a PackageID fails to perform an UninstallAction.
     *
     * \ingroup g_exceptions
     * \ingroup g_actions
     * \since 0.26
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE UninstallActionError : public ActionError
    {
        public:
            ///\name Basic operations
            ///\{

            UninstallActionError(const std::string & msg) throw ();

            ///\}
    };

    /**
     * Thrown if a PackageID fails to perform a ConfigAction.
     *
     * \ingroup g_exceptions
     * \ingroup g_actions
     * \since 0.26
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE ConfigActionError : public ActionError
    {
        public:
            ///\name Basic operations
            ///\{

            ConfigActionError(const std::string & msg) throw ();

            ///\}
    };

    /**
     * Thrown if a PackageID fails to perform an InfoAction.
     *
     * \ingroup g_exceptions
     * \ingroup g_actions
     * \since 0.26
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE InfoActionError : public ActionError
    {
        public:
            ///\name Basic operations
            ///\{

            InfoActionError(const std::string & msg) throw ();

            ///\}
    };
}

#endif
