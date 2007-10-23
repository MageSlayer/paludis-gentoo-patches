#!/bin/bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2006, 2007 Ciaran McCreesh
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

builtin_fetch()
{
    [[ -d "${FETCHEDDIR}" ]] || die "FETCHEDDIR \"${FETCHEDDIR}\" is not a directory"

    local a nofetch unique_aa old_aa
    for a in ${FLAT_SRC_URI} ; do
        local aa=${a##*/}
        hasq "${aa}" ${unique_aa} || unique_aa="${unique_aa} ${aa}"

        if [[ -f "${FETCHEDDIR}/${aa}" ]] && [[ "0" != $(wrapped_getfsize "${FETCHEDDIR}/${aa}") ]] ; then
            if [[ "${old_aa}" != "${aa}" ]] ; then
                ebuild_section "Already have ${aa}"
                old_aa="${aa}"
            fi
        else
            if [[ -f "${FETCHEDDIR}/${aa}" ]] ; then
                ebuild_section "Trying to remove existing ${aa}..."
                rm -f "${FETCHEDDIR}/${aa}"
            fi

            if ! hasq fetch ${RESTRICT} ; then
                if [[ "${old_aa}" != "${aa}" ]] ; then
                    ebuild_section "Need to fetch ${aa}"
                    old_aa="${aa}"
                fi
                local d
                for d in ${PALUDIS_FETCHERS_DIRS:-${PALUDIS_EBUILD_DIR}/fetchers/} ; do
                    prg=${d}/do$(echo ${a%%://*} | tr '[:upper:]' '[:lower:]' )
                    ebuild_notice "debug" "fetcher program candidate for '${a}' is '${prg}'"
                    [[ -x "${prg}" ]] && break
                done
                if [[ -x "${prg}" ]] ; then
                    ${prg} "${a}" "${FETCHEDDIR}/${aa}"
                else
                    eerror "Don't know how to fetch '${a}'"
                fi
            else
                if ! [[ "${old_aa}" != "${aa}" ]] ; then
                    ebuild_section "Can't fetch ${aa}"
                    old_aa="${aa}"
                fi
            fi
        fi
    done

    for a in ${unique_aa} ; do
        [[ -f ${FETCHEDDIR}/${a} ]] || nofetch="${nofetch} ${a}"
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

    local badfetch=
    if [[ -f "${FILESDIR}/digest-${PN}-${PVR%-r0}" ]] ; then
        local line items prg
        while read line ; do
            line=( ${line} )
            if ! hasq "${line[2]}" ${A} ; then
                ebuild_section "Skipping check for ${line[2]}"
                continue
            fi

            prg="${PALUDIS_EBUILD_DIR}/digests/do$(echo ${line[0]} | tr \
                '[[:upper:]]' '[[:lower:]]')"
            if [[ -x "${prg}" ]] ; then
                ebegin_unhooked "Checking ${line[0]} for ${line[2]}"
                if [[ $("${prg}" "${FETCHEDDIR}/${line[2]}" ) == "${line[1]}" ]] ; then
                    eend 0
                else
                    eend 1
                    hasq "${line[2]}" ${badfetch} || badfetch="${badfetch} ${line[2]}"
                fi
            else
                einfo_unhooked "Can't check ${line[0]} for ${line[2]}"
            fi

        done < "${FILESDIR}"/digest-${PN}-${PVR%-r0}
    else
        ebuild_section "No digest file, skipping integrity checks"
    fi

    if [[ -n "${badfetch}" ]] ; then
        local c
        echo
        eerror "Bad digests encountered for the following components:"
        for c in ${badfetch} ; do
            eerror "  * ${c}"
        done
        echo
        die "builtin_fetch failed"
    fi
}

ebuild_f_fetch()
{
    local old_sandbox_write="${SANDBOX_WRITE}"
    [[ -z "${PALUDIS_DO_NOTHING_SANDBOXY}" ]] && SANDBOX_WRITE="${SANDBOX_WRITE+${SANDBOX_WRITE}:}${FETCHEDDIR}"
    if hasq "fetch" ${RESTRICT} ; then
        ebuild_section "Skipping builtin_fetch (RESTRICT)"
    elif hasq "fetch" ${SKIP_FUNCTIONS} ; then
        ebuild_section "Skipping builtin_fetch (SKIP_FUNCTIONS)"
    else
        ebuild_section "Starting builtin_fetch"
        builtin_fetch
        ebuild_section "Done builtin_fetch"
    fi
    [[ -z "${PALUDIS_DO_NOTHING_SANDBOXY}" ]] && SANDBOX_WRITE="${old_sandbox_write}"
    true
}



