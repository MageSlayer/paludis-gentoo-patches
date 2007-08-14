/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Piotr Jaroszyński <peper@gentoo.org>
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
#include <paludis/util/tr1_memory.hh>
#include <paludis/repository.hh>

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
                    "NEED_DOC",
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
         "NEED_DOC");
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
            "Debug build mode for an InstallAction.");
    enum_auto("InstallActionChecksOption", last_iaco,
            "NEED_DOC");

    /**
     * InstallActionOptions
     */
    bp::class_<InstallActionOptions>
        (
         "InstallActionOptions",
         "Options for InstallAction.",
         bp::init<const bool &, const InstallActionDebugOption &, const InstallActionChecksOption &,
                const tr1::shared_ptr<paludis::Repository> &>(
                    "__init__(no_config_protect_bool, InstallActionDebugOption, "
                    "InstallActionChecksOption, Repository)"
                    )
        )
        .def_readwrite("no_config_protect", &InstallActionOptions::no_config_protect,
                "[rw] bool"
                )

        .def_readwrite("debug_build", &InstallActionOptions::debug_build,
                "[rw] InstallActionDebugOption"
                )

        .def_readwrite("checks", &InstallActionOptions::checks,
                "[rw] InstallActionChecksOption"
                )

        .add_property("destination",
                bp::make_getter(&InstallActionOptions::destination,
                    bp::return_value_policy<bp::return_by_value>()),
                bp::make_setter(&InstallActionOptions::destination),
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
        .def_readwrite("fetch_unneeded", &FetchActionOptions::fetch_unneeded,
                "[rw] bool"
                )

        .def_readwrite("safe_resume", &FetchActionOptions::safe_resume,
                "[rw] bool"
                )
        ;

    /**
     * UninstallActionOptions
     */
    bp::class_<UninstallActionOptions>
        (
         "UninstallActionOptions",
         "Options for UninstallAction.",
         bp::init<const bool &>("__init__(no_config_protect_bool)")
        )
        .def_readwrite("no_config_protect", &UninstallActionOptions::no_config_protect,
                "[rw] bool"
                )
        ;

   /**
     * Action
     */
    bp::class_<Action, boost::noncopyable>
        (
         "Action",
         "NEED_DOC",
         bp::no_init
        );

    /**
     * InstallAction
     */
    bp::class_<InstallAction, bp::bases<Action>, boost::noncopyable>
        (
         "InstallAction",
         "NEED_DOC",
         bp::init<const InstallActionOptions &>("__init__(InstallActionOptions)")
        );

    /**
     * FetchAction
     */
    bp::class_<FetchAction, bp::bases<Action>, boost::noncopyable>
        (
         "FetchAction",
         "NEED_DOC",
         bp::init<const FetchActionOptions &>("__init__(FetchActionOptions)")
        );

    /**
     * UninstallAction
     */
    bp::class_<UninstallAction, bp::bases<Action>, boost::noncopyable>
        (
         "UninstallAction",
         "NEED_DOC",
         bp::init<const UninstallActionOptions &>("__init__(UninstallActionOptions)")
        );

    /**
     * InsatlledAction
     */
    bp::class_<InstalledAction, bp::bases<Action>, boost::noncopyable>
        (
         "InstalledAction",
         "NEED_DOC",
         bp::init<>("__init__()")
        );

    /**
     * PretendAction
     */
    bp::class_<PretendAction, bp::bases<Action>, boost::noncopyable>
        (
         "PretendAction",
         "NEED_DOC",
         bp::init<>("__init__()")
        )
        .add_property("failed", &PretendAction::failed,
                "[ro] bool\n"
                "NEED_DOC"
                )
        ;

    /**
     * ConfigAction
     */
    bp::class_<ConfigAction, bp::bases<Action>, boost::noncopyable>
        (
         "ConfigAction",
         "NEED_DOC",
         bp::init<>("__init__()")
        );

    /**
     * SupportActionTestBase
     */
    bp::class_<SupportsActionTestBase, boost::noncopyable>
        (
         "SupportsActionTestBase",
         "NEED_DOC",
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

}
