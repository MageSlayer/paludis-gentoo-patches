/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009 Ciaran McCreesh
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
#include <paludis/util/simple_visitor.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/sequence-fwd.hh>
#include <paludis/util/named_value.hh>
#include <paludis/util/fs_entry-fwd.hh>
#include <paludis/util/output_manager-fwd.hh>
#include <paludis/util/type_list.hh>
#include <tr1/functional>

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
    namespace n
    {
        struct config_protect;
        struct destination;
        struct exclude_unmirrorable;
        struct failed_automatic_fetching;
        struct failed_integrity_checks;
        struct fetch_unneeded;
        struct make_output_manager;
        struct requires_manual_fetching;
        struct safe_resume;
        struct target_file;
        struct used_this_for_config_protect;
        struct want_phase;
    }

    /**
     * Options for a FetchAction.
     *
     * \see FetchAction
     * \ingroup g_actions
     * \since 0.30
     */
    struct FetchActionOptions
    {
        /**
         * \since 0.32
         */
        NamedValue<n::exclude_unmirrorable, bool> exclude_unmirrorable;

        NamedValue<n::fetch_unneeded, bool> fetch_unneeded;

        /**
         * This is a function to avoid chicken / egg problems when using
         * Environment::create_output_manager.
         *
         * \since 0.36
         */
        NamedValue<n::make_output_manager, std::tr1::function<std::tr1::shared_ptr<OutputManager> (
                const FetchAction &)> > make_output_manager;

        NamedValue<n::safe_resume, bool> safe_resume;
    };

    /**
     * Options for an InstallAction.
     *
     * \see InstallAction
     * \ingroup g_actions
     * \since 0.30
     */
    struct InstallActionOptions
    {
        NamedValue<n::destination, std::tr1::shared_ptr<Repository> > destination;

        /**
         * This is a function to avoid chicken / egg problems when using
         * Environment::create_output_manager.
         *
         * \since 0.36
         */
        NamedValue<n::make_output_manager, std::tr1::function<std::tr1::shared_ptr<OutputManager> (
                const InstallAction &)> > make_output_manager;

        NamedValue<n::used_this_for_config_protect, std::tr1::function<void (const std::string &)> > used_this_for_config_protect;
        NamedValue<n::want_phase, std::tr1::function<WantPhase (const std::string &)> > want_phase;
    };

    /**
     * Options for an UninstallAction.
     *
     * \see UninstallAction
     * \ingroup g_actions
     * \since 0.30
     */
    struct UninstallActionOptions
    {
        NamedValue<n::config_protect, std::string> config_protect;

        /**
         * This is a function to avoid chicken / egg problems when using
         * Environment::create_output_manager.
         *
         * \since 0.36
         */
        NamedValue<n::make_output_manager, std::tr1::function<std::tr1::shared_ptr<OutputManager> (
                const UninstallAction &)> > make_output_manager;
    };

    /**
     * A failed fetch action part.
     *
     * \see FetchActionError
     * \ingroup g_actions
     * \since 0.30
     */
    struct FetchActionFailure
    {
        NamedValue<n::failed_automatic_fetching, bool> failed_automatic_fetching;
        NamedValue<n::failed_integrity_checks, std::string> failed_integrity_checks;
        NamedValue<n::requires_manual_fetching, bool> requires_manual_fetching;
        NamedValue<n::target_file, std::string> target_file;
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
        public virtual DeclareAbstractAcceptMethods<Action, MakeTypeList<
            InstallAction, InstalledAction, UninstallAction, PretendAction, ConfigAction, FetchAction,
            InfoAction, PretendFetchAction>::Type>
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
        public ImplementAcceptMethods<Action, InstallAction>
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
        public ImplementAcceptMethods<Action, FetchAction>
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
        public ImplementAcceptMethods<Action, UninstallAction>
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
        public ImplementAcceptMethods<Action, InstalledAction>
    {
    };

    /**
     * Options for a PretendAction.
     *
     * \see PretendAction
     * \ingroup g_actions
     * \since 0.36
     */
    struct PretendActionOptions
    {
        /**
         * This is a function to avoid chicken / egg problems when using
         * Environment::create_output_manager.
         *
         * \since 0.36
         */
        NamedValue<n::make_output_manager, std::tr1::function<std::tr1::shared_ptr<OutputManager> (
                const PretendAction &)> > make_output_manager;
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
        public ImplementAcceptMethods<Action, PretendAction>
    {
        public:
            ///\name Basic operations
            ///\{

            /**
             * \since 0.36
             */
            PretendAction(const PretendActionOptions &);
            ~PretendAction();

            ///\}

            /// Did our pretend phase fail?
            bool failed() const PALUDIS_ATTRIBUTE((warn_unused_result));

            /// Mark the action as failed.
            void set_failed();

            /**
             * \since 0.36
             */
            const PretendActionOptions & options;
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
        public ImplementAcceptMethods<Action, PretendFetchAction>
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
     * Options for a ConfigAction.
     *
     * \see ConfigAction
     * \ingroup g_actions
     * \since 0.36
     */
    struct ConfigActionOptions
    {
        /**
         * This is a function to avoid chicken / egg problems when using
         * Environment::create_output_manager.
         *
         * \since 0.36
         */
        NamedValue<n::make_output_manager, std::tr1::function<std::tr1::shared_ptr<OutputManager> (
                const ConfigAction &)> > make_output_manager;
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
        private PrivateImplementationPattern<ConfigAction>,
        public ImplementAcceptMethods<Action, ConfigAction>
    {
        public:
            ///\name Basic operations
            ///\{

            /**
             * \since 0.36
             */
            ConfigAction(const ConfigActionOptions &);
            ~ConfigAction();

            ///\}

            /**
             * \since 0.36
             */
            const ConfigActionOptions & options;
    };

    /**
     * Options for an InfoAction.
     *
     * \see InfoAction
     * \ingroup g_actions
     * \since 0.36
     */
    struct InfoActionOptions
    {
        /**
         * This is a function to avoid chicken / egg problems when using
         * Environment::create_output_manager.
         *
         * \since 0.36
         */
        NamedValue<n::make_output_manager, std::tr1::function<std::tr1::shared_ptr<OutputManager> (
                const InfoAction &)> > make_output_manager;
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
        private PrivateImplementationPattern<InfoAction>,
        public ImplementAcceptMethods<Action, InfoAction>
    {
        public:
            ///\name Basic operations
            ///\{

            /**
             * \since 0.36
             */
            InfoAction(const InfoActionOptions &);

            ~InfoAction();

            ///\}

            /**
             * \since 0.36
             */
            const InfoActionOptions & options;
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
        public virtual DeclareAbstractAcceptMethods<SupportsActionTestBase, MakeTypeList<
            SupportsActionTest<InstallAction>, SupportsActionTest<InstalledAction>, SupportsActionTest<UninstallAction>,
            SupportsActionTest<PretendAction>, SupportsActionTest<ConfigAction>, SupportsActionTest<FetchAction>,
            SupportsActionTest<InfoAction>, SupportsActionTest<PretendFetchAction> >::Type>
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
        public ImplementAcceptMethods<SupportsActionTestBase, SupportsActionTest<A_> >
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

#ifdef PALUDIS_HAVE_EXTERN_TEMPLATE
    extern template class PrivateImplementationPattern<FetchAction>;
    extern template class PrivateImplementationPattern<InstallAction>;
    extern template class PrivateImplementationPattern<PretendAction>;
    extern template class PrivateImplementationPattern<PretendFetchAction>;
    extern template class PrivateImplementationPattern<UninstallAction>;
    extern template class PrivateImplementationPattern<InfoAction>;
    extern template class PrivateImplementationPattern<ConfigAction>;
#endif
}

#endif
