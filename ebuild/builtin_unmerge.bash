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

builtin_unmerge()
{
    local dbdir="${ROOT}"/var/db/pkg/"${CATEGORY}/${PF}" entry
    [[ -d "${dbdir}" ]] || die "couldn't find pkg db directory (\"${dbdir}\")"

    for v in CATEGORY CBUILD CFLAGS CHOST CXXFLAGS DEPEND DESCRIPTION EAPI \
        FEATURES HOMEPAGE INHERITED IUSE KEYWORDS LICENSE PDEPEND PF \
        PROVIDE RDEPEND SLOT SRC_URI USE ; do
        eval "${v}='$(< ${dbdir}/${v} )' || die \"Load key ${v} failed\""
    done

    sort -r -t ' ' -k 2 < "${dbdir}/CONTENTS" | \
    while read entry ; do
        [[ -z "${entry}" ]] && continue
        local items=( ${entry} )
        case ${items[0]} in
            dir)
                ;;

            sym)
                if ! [[ -L "${ROOT}/${items[1]}" ]] ; then
                    echo "skip  !type  ${items[1]}"
                elif [[ $(readlink "${ROOT}/${items[1]}" ) != "${items[3]}" ]] ; then
                    echo "skip  !dest  ${items[1]}"
                elif [[ $(stat -c '%Y' "${ROOT}/${items[1]}" ) != "${items[4]}" ]] ; then
                    echo "skip  !time  ${items[1]}"
                else
                    echo "remove       ${items[1]}"
                    rm -f "${ROOT}/${items[1]}"
                fi
                ;;

            obj)
                if ! [[ -f "${ROOT}/${items[1]}" ]] ; then
                    echo "skip  !type  ${items[1]}"
                elif [[ $(md5sum "${ROOT}/${items[1]}" | cut -d' ' -f1 ) != "${items[2]}" ]] ; then
                    echo "skip  !md5   ${items[1]}"
                elif [[ $(stat -c '%Y' "${ROOT}/${items[1]}" ) != "${items[3]}" ]] ; then
                    echo "skip  !time  ${items[1]}"
                else
                    echo "remove       ${items[1]}"
                    rm -f "${ROOT}/${items[1]}"
                fi
                ;;

            misc)
                if [[ -f "${ROOT}/${items[1]}" ]] ; then
                    echo "skip  !type  ${items[1]}"
                elif [[ $(stat -c '%Y' "${ROOT}/${items[1]}" ) != "${items[2]}" ]] ; then
                    echo "skip  !time  ${items[1]}"
                else
                    echo "remove       ${items[1]}"
                    rm -f "${ROOT}/${items[1]}"
                fi
                ;;

            *)
                die "Unknown VDB entry kind '${kind}'"
                ;;
        esac
    done

    shopt -q dotglob
    local olddotglob=$?
    shopt -s dotglob

    sort -r -t ' ' -k 2 < "${dbdir}/CONTENTS" | \
    while read entry ; do
        [[ -z "${entry}" ]] && continue
        local items=( ${entry} )
        case ${items[0]} in
            dir)
                if ! [[ -d "${ROOT}/${items[1]}" ]] ; then
                    echo "skip  !type  ${items[1]}"
                elif [[ $(echo "${ROOT}/${items[1]}"/* ) != "${ROOT}/${items[1]}/*" ]] ; then
                    echo "skip  !empty ${items[1]}"
                else
                    echo "remove       ${items[1]}"
                    rm -fr "${ROOT}/${items[1]}"
                fi
                ;;
        esac
    done

    [[ $olddotglob != 0 ]] && shopt -u dotglob
    shopt -q dotglob
    [[ $olddotglob == $? ]] || ebuild_notice "warning" "shopt dotglob restore failed"

    rm -fr "${dbdir}"
}

ebuild_f_unmerge()
{
    if hasq "unmerge" ${RESTRICT} ; then
        ebuild_section "Skipping builtin_unmerge (RESTRICT)"
    elif hasq "unmerge" ${SKIP_FUNCTIONS} ; then
        ebuild_section "Skipping builtin_unmerge (SKIP_FUNCTIONS)"
    else
        ebuild_section "Starting builtin_unmerge"
        builtin_unmerge
        ebuild_section "Done builtin_unmerge"
    fi
}


