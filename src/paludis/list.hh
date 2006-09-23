/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_SRC_LIST_REPOSITORIES_HH
#define PALUDIS_GUARD_SRC_LIST_REPOSITORIES_HH 1

/** \file
 * Declaration for the do_list_repositories and do_list_categories functions.
 */

/// Handle --list-repositories.
int do_list_repositories();

/// Handle --list-categories.
int do_list_categories();

/// Handle --list-packages.
int do_list_packages();

/// Handle --list-sets.
int do_list_sets();

/// Handle --list-vulnerabilities.
int do_list_vulnerabilities();

#endif
