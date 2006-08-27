#!/bin/bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
# Copyright (c) 2006 Stephen Bennett <spb@gentoo.org>
#
# This file is part of the Paludis package manager. Paludis is free software;
# you can redistribute it and/or modify it under the terms of the GNU General
# Public License as published by the Free Software Foundation; either version
# 2 of the License, or (at your option) any later version.
#
# Paludis is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 59 Temple
# Place, Suite 330, Boston, MA  02111-1307  USA

builtin_strip()
{
    STRIP=${STRIP:-${CHOST}-strip}
    if ! type -p -- ${STRIP} >/dev/null; then
        STRIP=strip
    fi
    PALUDIS_STRIP_FLAGS=${PALUDIS_STRIP_FLAGS:---strip-unneeded}

    for fn in $(find "${D}" -type f \( -perm -0100 -or -perm -0010 -or -perm -0001 -or -name '*.so' -or -name '*.so.*' \)); do
        local ft=$(file "${fn}")
        if [[ $? != 0 || -z ${ft} ]]; then
            return 1
        fi

        if [[ ${ft} == *"current ar archive"* ]]; then
            echo ${STRIP} -g "${fn}"
            ${STRIP} -g "${fn}"
        elif [[ ${ft} == *"SB executable"* || ${ft} == *"SB shared object"* ]]; then
            echo ${STRIP} ${PALUDIS_STRIP_FLAGS} "${fn}"
            ${STRIP} ${PALUDIS_STRIP_FLAGS} "${fn}"
        fi
    done
}

ebuild_f_strip()
{
    if hasq "strip" ${RESTRICT} ; then
        ebuild_section "Skipping builtin_strip (RESTRICT)"
    elif hasq "strip" ${SKIP_FUNCTIONS} ; then
        ebuild_section "Skipping builtin_strip (SKIP_FUNCTIONS)"
    else
        ebuild_section "Starting builtin_strip"
        builtin_strip
        ebuild_section "Done builtin_strip"
    fi
}

