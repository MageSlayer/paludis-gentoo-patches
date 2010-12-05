#!/usr/bin/env bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2007, 2008, 2010 Ciaran McCreesh
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

default_builtin_bad_required_use()
{
    eerror "The following required use constraints are unmet for ${CATEGORY}/${PF}:"
    local f
    echo "${EX_UNMET_REQUIREMENTS}" | while IFS=$'\n' read f ; do
        eerror "    ${f}"
    done
}

builtin_bad_required_use()
{
    default "$@"
}

ebuild_f_bad_required_use()
{
    if hasq "bad_required_use" ${SKIP_FUNCTIONS} ; then
        ebuild_section "Skipping builtin_bad_required_use (SKIP_FUNCTIONS)"
    else
        echo
        builtin_bad_required_use
        echo
    fi

    true
}

