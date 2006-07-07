/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#include "src/command_line.hh"

/** \file
 * Declaration for small do_* functions.
 */

/// Handle --has-version.
int do_has_version();

/// Handle --best-version.
int do_best_version();

/// Handle --environment-variable.
int do_environment_variable();

/// Handle --configuration-variable.
int do_configuration_variable();

/// Handle --list-repository-formats
int do_list_repository_formats();

/// Handle --list-sync-protocols
int do_list_sync_protocols();

/// Handle --list-dep-tag-categories
int do_list_dep_tag_categories();

#endif

