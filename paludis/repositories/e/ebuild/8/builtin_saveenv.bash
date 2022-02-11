#!/usr/bin/env bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2007 Ciaran McCreesh
# Copyright (c) 2022 Mihai Moldovan
#
# This file is part of the Paludis package manager. Paludis is free software;
# you can redistribute it and/or modify it under the terms of the GNU General
# Public License, version 2, as published by the Free Software Foundation.
#
# Paludis is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 59 Temple
# Place, Suite 330, Boston, MA  02111-1307  USA

ebuild_load_module --older builtin_saveenv

ebuild_f_saveenv()
{
    local old_sandbox_write="${SANDBOX_WRITE}"
    SANDBOX_WRITE="${SANDBOX_WRITE+${SANDBOX_WRITE}:}${PALUDIS_LOADSAVEENV_DIR%/}/"

    if has "saveenv" ${SKIP_FUNCTIONS} ; then
        ebuild_section "Skipping builtin_saveenv (SKIP_FUNCTIONS)"
    else
        ebuild_section "Starting builtin_saveenv"
        builtin_saveenv
        ebuild_section "Done builtin_saveenv"
    fi

    SANDBOX_WRITE="${old_sandbox_write}"
    true
}
