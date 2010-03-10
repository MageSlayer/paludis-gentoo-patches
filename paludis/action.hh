/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010 Ciaran McCreesh
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
#include <paludis/util/options.hh>
#include <paludis/output_manager-fwd.hh>
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
        struct errors;
        struct exclude_unmirrorable;
        struct failed_automatic_fetching;
        struct failed_integrity_checks;
        struct fetch_parts;
        struct if_for_install_id;
        struct ignore_for_unmerge;
        struct ignore_unfetched;
        struct is_overwrite;
        struct make_output_manager;
        struct perform_uninstall;
        struct replacing;
        struct requires_manual_fetching;
        struct safe_resume;
        struct target_file;
        struct want_phase;
        struct ignore_not_in_manifest;
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
         * Any errors that occur will be added to this list. Must not be null.
         *
         * \since 0.40
         */
        NamedValue<n::errors, std::tr1::shared_ptr<Sequence<FetchActionFailure> > > errors;

        /**
         * \since 0.32
         */
        NamedValue<n::exclude_unmirrorable, bool> exclude_unmirrorable;

        /**
         * Which parts to fetch.
         *
         * \since 0.43
         */
        NamedValue<n::fetch_parts, FetchParts> fetch_parts;

        /**
         * Ignore any unfetched packages. Verify digests for anything that's
         * already there, and if we know for sure manual fetching will be
         * required, raise the appropriate error.
         *
         * \since 0.36
         */
        NamedValue<n::ignore_unfetched, bool> ignore_unfetched;

        /**
         * This is a function to avoid chicken / egg problems when using
         * Environment::create_output_manager.
         *
         * \since 0.36
         */
        NamedValue<n::make_output_manager, std::tr1::function<std::tr1::shared_ptr<OutputManager> (
                const FetchAction &)> > make_output_manager;


        NamedValue<n::safe_resume, bool> safe_resume;

         /**
          * Ignore if a package is or isn't referenced in the Manifest.
          * It's useful with appareo not to get errors before generating it.
          *
          * \since 0.46
          */
        NamedValue<n::ignore_not_in_manifest, bool> ignore_not_in_manifest;
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

        /**
         * Callback to carry out an uninstall, for replacing.
         *
         * Won't necessarily be used. Some repositories have special code paths
         * for reinstalls, and in some cases (e.g. accounts) an upgrade doesn't
         * remove the old version at all.
         *
         * \since 0.36
         */
        NamedValue<n::perform_uninstall, std::tr1::function<void (
                const std::tr1::shared_ptr<const PackageID> &,
                const UninstallActionOptions &
                )> > perform_uninstall;

        /**
         * We must replace these.
         *
         * \since 0.36
         */
        NamedValue<n::replacing, std::tr1::shared_ptr<const PackageIDSequence> > replacing;

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
         * If we're being uninstalled as part of an install, this is the ID
         * that's being installed. Otherwise null.
         *
         * \since 0.36
         */
        NamedValue<n::if_for_install_id, std::tr1::shared_ptr<const PackageID> > if_for_install_id;

        /**
         * Sometimes we never want to unmerge certain files.
         *
         * \since 0,38
         */
        NamedValue<n::ignore_for_unmerge, std::tr1::function<bool (const FSEntry &)> > ignore_for_unmerge;

        /**
         * Some repositories need to do special handlings for direct overwrites
         * (foo-1.2 replacing foo-1.2). Clients should set this to false.
         *
         * \since 0.36
         */
        NamedValue<n::is_overwrite, bool> is_overwrite;

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
            InstallAction, UninstallAction, PretendAction, ConfigAction, FetchAction,
            InfoAction, PretendFetchAction>::Type>
    {
        public:
            ///\name Basic operations
            ///\{

            virtual ~Action() = 0;

            ///\}

            /**
             * A simple string name (install, uninstall, pretend-fetch etc).
             *
             * \since 0.44
             */
            virtual const std::string simple_name() const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
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

            virtual const std::string simple_name() const PALUDIS_ATTRIBUTE((warn_unused_result));

            static const std::string class_simple_name() PALUDIS_ATTRIBUTE((warn_unused_result));
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

            virtual const std::string simple_name() const PALUDIS_ATTRIBUTE((warn_unused_result));

            static const std::string class_simple_name() PALUDIS_ATTRIBUTE((warn_unused_result));
            static const std::string ignore_unfetched_flag_name() PALUDIS_ATTRIBUTE((warn_unused_result));
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

            virtual const std::string simple_name() const PALUDIS_ATTRIBUTE((warn_unused_result));

            static const std::string class_simple_name() PALUDIS_ATTRIBUTE((warn_unused_result));
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

            virtual const std::string simple_name() const PALUDIS_ATTRIBUTE((warn_unused_result));

            static const std::string class_simple_name() PALUDIS_ATTRIBUTE((warn_unused_result));
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

            virtual const std::string simple_name() const PALUDIS_ATTRIBUTE((warn_unused_result));

            static const std::string class_simple_name() PALUDIS_ATTRIBUTE((warn_unused_result));
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

            virtual const std::string simple_name() const PALUDIS_ATTRIBUTE((warn_unused_result));

            static const std::string class_simple_name() PALUDIS_ATTRIBUTE((warn_unused_result));
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

            virtual const std::string simple_name() const PALUDIS_ATTRIBUTE((warn_unused_result));

            static const std::string class_simple_name() PALUDIS_ATTRIBUTE((warn_unused_result));
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
            SupportsActionTest<InstallAction>, SupportsActionTest<UninstallAction>,
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
     * Thrown if an action fails.
     *
     * \ingroup g_actions
     * \ingroup g_exceptions
     * \since 0.42
     */
    class PALUDIS_VISIBLE ActionFailedError :
        public Exception
    {
        public:
            ///\name Basic operations
            ///\{

            ActionFailedError(const std::string & msg) throw ();

            ///\}
    };

    /**
     * Thrown if an action is aborted.
     *
     * \ingroup g_actions
     * \ingroup g_exceptions
     * \since 0.42
     */
    class PALUDIS_VISIBLE ActionAbortedError :
        public Exception
    {
        public:
            ///\name Basic operations
            ///\{

            ActionAbortedError(const std::string &) throw ();

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
