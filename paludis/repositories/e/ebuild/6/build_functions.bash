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
                patch -p1 -f -g0 --no-backup-if-mismatch "${options[@]}" < "${f}"
                st=${?}
                if [[ ${st} -ne 0 ]] ; then
                    paludis_die_unless_nonfatal "applying patch ${f} failed"
                    return ${st}
                fi
            done

            [[ -z ${any} ]] && die "no patches found in directory ${x}"

        else
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

