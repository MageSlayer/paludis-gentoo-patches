#!/usr/bin/env bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2006, 2007, 2009 Ciaran McCreesh
# Copyright (c) 2015 David Leverton
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

ebuild_load_module --older build_functions

eapply()
{
    local -a options files
    local p dashdash= badmix=
    for p in "${@}" ; do
        if [[ -n ${dashdash} ]] ; then
            files+=( "${p}" )
        elif [[ ${p} == -- ]] ; then
            options+=( "${files[@]}" )
            files=( )
            dashdash=yes
        elif [[ ${p} == -* && ${#files[@]} -eq 0 ]] ; then
            options+=( "${p}" )
        else
            [[ ${p} == -* ]] && badmix=yes
            files+=( "${p}" )
        fi
    done

    [[ -z ${dashdash} && -n ${badmix} ]] && die "options must be specified before patches"
    [[ ${#files[@]} -eq 0 ]] && die "no patches specified"

    local x st
    for x in "${files[@]}" ; do
        if [[ -d ${x} ]] ; then
            local f any=

            for f in "${x}"/*.@(diff|patch) ; do
                [[ -e ${f} ]] || continue
                any=yes
                echo patch -p1 -f -g0 --no-backup-if-mismatch "${options[@]}" \< "${f}" 1>&2
                patch -p1 -f -g0 --no-backup-if-mismatch "${options[@]}" < "${f}"
                st=${?}
                if [[ ${st} -ne 0 ]] ; then
                    paludis_die_unless_nonfatal "applying patch ${f} failed"
                    return ${st}
                fi
            done

            [[ -z ${any} ]] && die "no patches found in directory ${x}"

        else
            echo patch -p1 -f -g0 --no-backup-if-mismatch "${options[@]}" \< "${x}" 1>&2
            patch -p1 -f -g0 --no-backup-if-mismatch "${options[@]}" < "${x}"
            st=${?}
            if [[ ${st} -ne 0 ]] ; then
                paludis_die_unless_nonfatal "applying patch ${x} failed"
                return ${st}
            fi
        fi
    done

    return 0
}
ebuild_need_extglob eapply

eapply_user()
{
    : ${EPATCH_USER_SOURCE:=${PALUDIS_USER_PATCHES%/}}
    [[ $# -ne 0 ]] && die "eapply_user takes no options"

    # Allow multiple calls to this function; ignore all but the first
    local applied="${T}/eapply_user.log"
    [[ -e ${applied} ]] && return 2

    # don't clobber any EPATCH vars that the parent might want
    local EPATCH_SOURCE check
    for check in ${CATEGORY}/{${P}-${PR},${P},${PN}}{,:${SLOT%/*}}; do
        EPATCH_SOURCE=""
        # CTARGET might be empty, let's avoid double slashes...
        [[ -n ${CTARGET} ]] && EPATCH_SOURCE="${EPATCH_USER_SOURCE}/${CTARGET}/${check}"
        [[ -r ${EPATCH_SOURCE} ]] || EPATCH_SOURCE="${EPATCH_USER_SOURCE}/${CHOST}/${check}"
        [[ -r ${EPATCH_SOURCE} ]] || EPATCH_SOURCE="${EPATCH_USER_SOURCE}/${check}"
        if [[ -d ${EPATCH_SOURCE} ]] ; then
            EPATCH_SOURCE="${EPATCH_SOURCE}" \
            EPATCH_SUFFIX="patch" \
            EPATCH_FORCE="yes" \
            EPATCH_MULTI_MSG="Applying user patches from ${EPATCH_SOURCE} ..." \
            epatch
            echo "${EPATCH_SOURCE}" > "${applied}"
            return 0
        fi
    done
    echo "none" > "${applied}"
    return 1
}

einstall()
{
    die "einstall is banned in EAPI 6"
}

einstalldocs()
{
    local DOCDESTTREE=
    if ! declare -p DOCS >/dev/null 2>&1 ; then
        local d
        for d in README* ChangeLog AUTHORS NEWS TODO CHANGES \
            THANKS BUGS FAQ CREDITS CHANGELOG ; do
            if [[ -s "${d}" ]] ; then
                dodoc "${d}" || return $?
            fi
        done
    elif declare -p DOCS | grep -q '^declare -a ' ; then
        if [[ ${#DOCS[@]} -gt 0 ]] ; then
            dodoc -r "${DOCS[@]}" || return $?
        fi
    else
        if [[ -n ${DOCS} ]] ; then
            dodoc -r ${DOCS} || return $?
        fi
    fi

    DOCDESTTREE=html
    if ! declare -p HTML_DOCS >/dev/null 2>&1 ; then
        :
    elif declare -p HTML_DOCS | grep -q '^declare -a ' ; then
        if [[ ${#HTML_DOCS[@]} -gt 0 ]] ; then
            dodoc -r "${HTML_DOCS[@]}" || return $?
        fi
    else
        if [[ -n ${HTML_DOCS} ]] ; then
            dodoc -r ${HTML_DOCS} || return $?
        fi
    fi

    return 0
}

