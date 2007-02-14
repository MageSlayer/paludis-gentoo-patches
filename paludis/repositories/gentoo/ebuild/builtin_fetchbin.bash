#!/bin/bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

builtin_fetchbin()
{
    [[ -d "${PKGDIR}" ]] || die "PKGDIR \"${PKGDIR}\" is not a directory"

    local a nofetch unique_aa old_aa
    for a in ${FLAT_BIN_URI} ; do
        local aa=${a##*/}
        hasq "${aa}" ${unique_aa} || unique_aa="${unique_aa} ${aa}"

        if [[ -f "${PKGDIR}/${aa}" ]] && [[ "0" != $(getfsize "${PKGDIR}/${aa}") ]] ; then
            if [[ "${old_aa}" != "${aa}" ]] ; then
                ebuild_section "Already have ${aa}"
                old_aa="${aa}"
            fi
        else
            if [[ -f "${PKGDIR}/${aa}" ]] ; then
                ebuild_section "Trying to remove existing ${aa}..."
                rm -f "${PKGDIR}/${aa}"
            fi

            if ! hasq fetchbin ${RESTRICT} ; then
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
                    ${prg} "${a}" "${PKGDIR}/${aa}"
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
        [[ -f ${PKGDIR}/${a} ]] || nofetch="${nofetch} ${a}"
    done

    if [[ -n "${nofetch}" ]] ; then
        local c
        echo
        eerror "Couldn't fetch the following components:"
        for c in ${nofetch} ; do
            eerror "  * ${c}"
        done
        echo
        die "builtin_fetchbin failed"
    fi
}

ebuild_f_fetchbin()
{
    local old_sandbox_write="${SANDBOX_WRITE}"
    [[ -z "${PALUDIS_DO_NOTHING_SANDBOXY}" ]] && SANDBOX_WRITE="${SANDBOX_WRITE+${SANDBOX_WRITE}:}${PKGDIR}"
    if hasq "fetchbin" ${RESTRICT} ; then
        ebuild_section "Skipping builtin_fetchbin (RESTRICT)"
    elif hasq "fetchbin" ${SKIP_FUNCTIONS} ; then
        ebuild_section "Skipping builtin_fetchbin (SKIP_FUNCTIONS)"
    else
        ebuild_section "Starting builtin_fetchbin"
        builtin_fetchbin
        ebuild_section "Done builtin_fetchbin"
    fi
    [[ -z "${PALUDIS_DO_NOTHING_SANDBOXY}" ]] && SANDBOX_WRITE="${old_sandbox_write}"
    true
}



