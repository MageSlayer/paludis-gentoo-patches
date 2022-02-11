#!/usr/bin/env bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2006, 2007, 2008 Ciaran McCreesh
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

ebuild_load_module --older builtin_initmisc

ebuild_f_initmisc()
{
    if has "initmisc" ${SKIP_FUNCTIONS} ; then
        ebuild_section "Skipping builtin_initmisc (SKIP_FUNCTIONS)"
    else
        ebuild_section "Starting builtin_initmisc"
        builtin_initmisc
        ebuild_section "Done builtin_initmisc"
    fi
}
