/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_SRC_APPLETS_HH
#define PALUDIS_GUARD_SRC_APPLETS_HH 1

#include "command_line.hh"

/** \file
 * Declaration for small do_* functions.
 */

namespace paludis
{
    class Environment;
}

/// Handle --has-version.
int do_has_version(const std::shared_ptr<paludis::Environment> &);

/// Handle --best-version.
int do_best_version(const std::shared_ptr<paludis::Environment> &);

/// Handle --match.
int do_match(const std::shared_ptr<paludis::Environment> &);

/// Handle --environment-variable.
int do_environment_variable(const std::shared_ptr<paludis::Environment> &);

/// Handle --configuration-variable.
int do_configuration_variable(const std::shared_ptr<paludis::Environment> &);

/// Handle --list-repository-formats
int do_list_repository_formats();

/// Handle --list-sync-protocols
int do_list_sync_protocols(const std::shared_ptr<paludis::Environment> &);

/// Handle cache regeneration
int do_regenerate_cache(const std::shared_ptr<paludis::Environment> &, bool installed);

#endif

