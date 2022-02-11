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

ebuild_load_module --older builtin_infovars

ebuild_f_infovars()
{
    if has "infovars" ${SKIP_FUNCTIONS} ; then
        ebuild_section "Skipping builtin_infovars (SKIP_FUNCTIONS)"
    else
        if [[ $(type -t pre_builtin_infovars ) == "function" ]] ; then
            ebuild_section "Starting pre_builtin_infovars"
            pre_builtin_infovars
            ebuild_section "Done pre_builtin_infovars"
        fi

        ebuild_section "Starting builtin_infovars"
        builtin_infovars
        ebuild_section "Done builtin_infovars"

        if [[ $(type -t post_builtin_infovars ) == "function" ]] ; then
            ebuild_section "Starting post_builtin_infovars"
            post_builtin_infovars
            ebuild_section "Done post_builtin_infovars"
        fi
    fi

    true
}
