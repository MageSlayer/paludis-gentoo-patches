/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Piotr Jaroszyński
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

#include <python/paludis_python.hh>
#include <python/exception.hh>

#include <paludis/action.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/kc.hh>
#include <paludis/repository.hh>
#include <tr1/memory>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

template <typename A_>
class class_supports_action_test :
    public bp::class_<SupportsActionTest<A_>, bp::bases<SupportsActionTestBase> >
{
    public:
        class_supports_action_test(const std::string & action) :
            bp::class_<SupportsActionTest<A_>, bp::bases<SupportsActionTestBase> >(
                    ("Supports" + action + "ActionTest").c_str(),
                    "SupportsActionTest is used by PackageID::supports_action and\n"
                    "Repository::some_ids_might_support_action to query whether a\n"
                    "particular action is supported by that PackageID or potentially\n"
                    "supported by some IDs in that Repository.",
                    bp::init<>("__init__()")
                    )
        {
        }
};

void expose_action()
{
    /**
     * Exceptions
     */
    ExceptionRegister::get_instance()->add_exception<ActionError>
        ("ActionError", "BaseException",
         "Parent class for action errors.");
    ExceptionRegister::get_instance()->add_exception<UnsupportedActionError>
        ("UnsupportedActionError", "ActionError",
         "Thrown if a PackageID is asked to perform an Action that it does not support.");
    ExceptionRegister::get_instance()->add_exception<InstallActionError>
        ("InstallActionError", "ActionError",
         "Thrown if an install fails.");
    ExceptionRegister::get_instance()->add_exception<FetchActionError>
        ("FetchActionError", "ActionError",
         "Thrown if a fetch fails.");
    ExceptionRegister::get_instance()->add_exception<UninstallActionError>
        ("UninstallActionError", "ActionError",
         "Thrown if an uninstall fails.");
    ExceptionRegister::get_instance()->add_exception<ConfigActionError>
        ("ConfigActionError", "ActionError",
         "Thrown if a configure fails.");

    /**
     * Enums
     */
    enum_auto("InstallActionDebugOption", last_iado,
            "Debug build mode for an InstallAction.\n\n"
            "May be ignored by some repositories, and by packages where there\n"
            "isn't a sensible concept of debugging.");

    enum_auto("InstallActionChecksOption", last_iaco,
            "Whether to run post-build checks (for example, 'make check' or 'src_test'),\n"
            "if they are available.");

    /**
     * InstallActionOptions
     */
    bp::class_<InstallActionOptions>
        (
         "InstallActionOptions",
         "Options for InstallAction.",
         bp::init<const InstallActionDebugOption &, const InstallActionChecksOption &,
                const std::tr1::shared_ptr<paludis::Repository> &>(
                    "__init__(InstallActionDebugOption, InstallActionChecksOption, Repository)"
                    )
        )

        .add_property("debug_build",
                &kc_getter<InstallActionOptions, InstallActionDebugOption, k::debug_build>,
                &kc_setter<InstallActionOptions, InstallActionDebugOption, k::debug_build>,
                "[rw] InstallActionDebugOption"
                )

        .add_property("checks",
                &kc_getter<InstallActionOptions, InstallActionChecksOption, k::checks>,
                &kc_setter<InstallActionOptions, InstallActionChecksOption, k::checks>,
                "[rw] InstallActionChecksOption"
                )

        .add_property("destination",
                &kc_getter<InstallActionOptions, std::tr1::shared_ptr<Repository>, k::destination>,
                &kc_setter<InstallActionOptions, std::tr1::shared_ptr<Repository>, k::destination>,
                "[rw] Repository"
                )
        ;

    /**
     * FetchActionOptions
     */
    bp::class_<FetchActionOptions>
        (
         "FetchActionOptions",
         "Options for FetchAction.",
         bp::init<const bool &, const bool &>("__init__(fetch_unneeded_bool, safe_resume_bool)")
        )
        .add_property("fetch_unneeded",
                &kc_getter<FetchActionOptions, bool, k::fetch_unneeded>,
                &kc_setter<FetchActionOptions, bool, k::fetch_unneeded>,
                "[rw] bool"
                )

        .add_property("safe_resume",
                &kc_getter<FetchActionOptions, bool, k::safe_resume>,
                &kc_setter<FetchActionOptions, bool, k::safe_resume>,
                "[rw] bool"
                )
        ;

   /**
     * Action
     */
    bp::class_<Action, boost::noncopyable>
        (
         "Action",
         "An Action represents an action that can be executed by a PackageID via\n"
         "PackageID::perform_action.",
         bp::no_init
        );

    /**
     * InstallAction
     */
    bp::class_<InstallAction, bp::bases<Action>, boost::noncopyable>
        (
         "InstallAction",
         "An InstallAction is used by InstallTask to perform a build / install on a\n"
         "PackageID.",
         bp::init<const InstallActionOptions &>("__init__(InstallActionOptions)")
        );

    /**
     * FetchAction
     */
    bp::class_<FetchAction, bp::bases<Action>, boost::noncopyable>
        (
         "FetchAction",
         "A FetchAction can be used to fetch source files for a PackageID using\n"
         "PackageID::perform_action.",
         bp::init<const FetchActionOptions &>("__init__(FetchActionOptions)")
        );

    /**
     * UninstallAction
     */
    bp::class_<UninstallAction, bp::bases<Action>, boost::noncopyable>
        (
         "UninstallAction",
         "An UninstallAction is used by UninstallTask to uninstall a PackageID.",
         bp::init<>("__init__()")
        );

    /**
     * InsatlledAction
     */
    bp::class_<InstalledAction, bp::bases<Action>, boost::noncopyable>
        (
         "InstalledAction",
         "InstalledAction is a dummy action used by SupportsActionTest and\n"
         "query::SupportsAction to determine whether a PackageID is installed.",
         bp::init<>("__init__()")
        );

    /**
     * PretendAction
     */
    bp::class_<PretendAction, bp::bases<Action>, boost::noncopyable>
        (
         "PretendAction",
         "A PretendAction is used by InstallTask to handle install-pretend-phase\n"
         "checks on a PackageID.",
         bp::init<>("__init__()")
        )
        .add_property("failed", &PretendAction::failed,
                "[ro] bool\n"
                "Did our pretend phase fail?"
                )
        ;

    /**
     * ConfigAction
     */
    bp::class_<ConfigAction, bp::bases<Action>, boost::noncopyable>
        (
         "ConfigAction",
         "A ConfigAction is used via PackageID::perform_action to execute\n"
         "post-install configuration (for example, via 'paludis --config')\n"
         "on a PackageID.",
         bp::init<>("__init__()")
        );

    /**
     * InfoAction
     */
    bp::class_<InfoAction, bp::bases<Action>, boost::noncopyable>
        (
         "InfoAction",
         "An InfoAction is used via PackageID::perform_action to execute\n"
         "additional information (for example, via 'paludis --info')\n"
         "on a PackageID.",
         bp::init<>("__init__()")
        );

    /**
     * SupportActionTestBase
     */
    bp::class_<SupportsActionTestBase, boost::noncopyable>
        (
         "SupportsActionTestBase",
         "Base class for SupportsActionTests.",
         bp::no_init
        );

    /**
     * SupportActionTests
     */
    class_supports_action_test<InstallAction>("Install");
    class_supports_action_test<FetchAction>("Fetch");
    class_supports_action_test<UninstallAction>("Uninstall");
    class_supports_action_test<InstalledAction>("Installed");
    class_supports_action_test<PretendAction>("Pretend");
    class_supports_action_test<ConfigAction>("Config");
    class_supports_action_test<InfoAction>("Info");

}
