#!/bin/bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

builtin_fetch()
{
    local a nofetch unique_aa
    for a in ${FLAT_SRC_URI} ; do
        local aa=${a##*/}
        if [[ -f "${DISTDIR}/${aa}" ]] ; then
            if ! hasq ${aa} ${unique_aa} ; then
                ebuild_section "Already have ${aa}"
                unique_aa="${unique_aa} ${aa}"
            fi
            continue
        else
            if ! hasq fetch ${RESTRICT} ; then
                if ! hasq ${aa} ${unique_aa} ; then
                    ebuild_section "Need to fetch ${aa}"
                    unique_aa="${unique_aa} ${aa}"
                fi
                wget -O "${DISTDIR}/${aa}" "${a}"
            else
                if ! hasq ${aa} ${unique_aa} ; then
                    ebuild_section "Can't fetch ${aa}"
                    unique_aa="${unique_aa} ${aa}"
                fi
                nofetch="${nofetch:+${nofetch} }${aa}"
            fi
        fi
    done

    if [[ -n "${nofetch}" ]] ; then
        local c
        echo
        eerror "Couldn't fetch the following components:"
        for c in ${nofetch} ; do
            eerror "  * ${c}"
        done
        echo
        die "builtin_fetch failed"
    fi
}

ebuild_f_fetch()
{
    if hasq "fetch" ${RESTRICT} ; then
        ebuild_section "Skipping builtin_fetch (RESTRICT)"
    elif hasq "fetch" ${SKIP_FUNCTIONS} ; then
        ebuild_section "Skipping builtin_fetch (SKIP_FUNCTIONS)"
    else
        ebuild_section "Starting builtin_fetch"
        builtin_fetch
        ebuild_section "Done builtin_fetch"
    fi
}


