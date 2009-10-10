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

#ifndef PALUDIS_GUARD_PALUDIS_ACTION_FWD_HH
#define PALUDIS_GUARD_PALUDIS_ACTION_FWD_HH 1

#include <paludis/util/attributes.hh>
#include <tr1/memory>
#include <iosfwd>

/** \file
 * Forward declarations for paludis/action.hh .
 *
 * \ingroup g_actions
 */

namespace paludis
{
    class Action;
    class InstallAction;
    class UninstallAction;
    class PretendAction;
    class ConfigAction;
    class FetchAction;
    class InfoAction;
    class PretendFetchAction;

    class SupportsActionTestBase;
    template <typename A_> class SupportsActionTest;

    class ActionFailedError;
    class ActionAbortedError;

    struct FetchActionOptions;
    struct InstallActionOptions;
    struct UninstallActionOptions;
    struct InfoActionOptions;
    struct ConfigActionOptions;
    struct PretendActionOptions;

    struct FetchActionFailure;

#include <paludis/action-se.hh>

}

#endif
