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

#ifndef PALUDIS_GUARD_PALUDIS_ACTION_FWD_HH
#define PALUDIS_GUARD_PALUDIS_ACTION_FWD_HH 1

#include <iosfwd>
#include <paludis/util/attributes.hh>
#include <paludis/util/kc-fwd.hh>
#include <paludis/util/keys.hh>
#include <paludis/repository-fwd.hh>
#include <tr1/memory>

/** \file
 * Forward declarations for paludis/action.hh .
 *
 * \ingroup g_actions
 */

namespace paludis
{
    class Action;
    class InstallAction;
    class InstalledAction;
    class UninstallAction;
    class PretendAction;
    class ConfigAction;
    class FetchAction;
    class InfoAction;
    class PretendFetchAction;

    class SupportsActionTestBase;
    template <typename A_> class SupportsActionTest;

    class ActionVisitorTypes;
    class SupportsActionTestVisitorTypes;

    class ActionError;
    class UnsupportedActionError;
    class InstallActionError;
    class FetchActionError;
    class UninstallActionError;
    class ConfigActionError;
    class InfoActionError;

#include <paludis/action-se.hh>

    /**
     * Options for a FetchAction.
     *
     * \see FetchAction
     * \ingroup g_actions
     * \since 0.26
     */
    typedef kc::KeyedClass<
        kc::Field<k::fetch_unneeded, bool>,
        kc::Field<k::safe_resume, bool>
            > FetchActionOptions;

    /**
     * Options for an InstallAction.
     *
     * \see InstallAction
     * \ingroup g_actions
     * \since 0.26
     */
    typedef kc::KeyedClass<
        kc::Field<k::debug_build, InstallActionDebugOption>,
        kc::Field<k::checks, InstallActionChecksOption>,
        kc::Field<k::destination, std::tr1::shared_ptr<Repository> >
            > InstallActionOptions;

    /**
     * A failed fetch action part.
     *
     * \see FetchActionError
     * \ingroup g_actions
     * \since 0.26
     */
    typedef kc::KeyedClass<
        kc::Field<k::target_file, std::string>,
        kc::Field<k::requires_manual_fetching, bool>,
        kc::Field<k::failed_automatic_fetching, bool>,
        kc::Field<k::failed_integrity_checks, std::string>
            > FetchActionFailure;

}

#endif
