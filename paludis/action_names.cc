/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009 Ciaran McCreesh
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

#include <paludis/action_names.hh>

using namespace paludis;

const char * const ActionNames<InstallAction>::value = "install";
const char * const ActionNames<UninstallAction>::value = "uninstall";
const char * const ActionNames<ConfigAction>::value = "config";
const char * const ActionNames<PretendAction>::value = "pretend";
const char * const ActionNames<InfoAction>::value = "info";
const char * const ActionNames<FetchAction>::value = "fetch";
const char * const ActionNames<PretendFetchAction>::value = "pretend fetch";

