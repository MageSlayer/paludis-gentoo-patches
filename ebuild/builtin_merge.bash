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

builtin_merge()
{
    if [[ -n "${CONFIG_PROTECT}" ]] ; then
        local d f
        for d in ${CONFIG_PROTECT} ; do

            find "${D}${d}" -type f | \
            while read f ; do
                if [[ -e "${ROOT}${f#${D}}" ]] ; then
                    echo "CONFIG_PROTECT ${f#${D}}"
                fi
            done
        done
    fi

    install -d "${ROOT}/" || die "couldn't make \${ROOT} (\"${ROOT}\")"
    if [[ -d "${D}" ]] ; then
        cp -vdfpR "${D}/"* "${ROOT}/" || die "builtin_merge failed"
    fi

    local dbdir="${ROOT}"/var/db/pkg/"${CATEGORY}/${PF}"
    install -d "${dbdir}" || die "couldn't make pkg db directory (\"${dbdir}\")"

    local v
    for v in CATEGORY CBUILD CFLAGS CHOST CXXFLAGS DEPEND DESCRIPTION EAPI \
        FEATURES HOMEPAGE INHERITED IUSE KEYWORDS LICENSE PDEPEND PF \
        PROVIDE RDEPEND SLOT SRC_URI USE ; do
        echo "${!v}" > "${dbdir}"/${v} || die "pkg db write ${v} failed"
    done

    [[ -f "${EBUILD}" ]] && cp "${EBUILD}" ${dbdir}/
    env | bzip2 > ${dbdir}/environment.bz2

    touch ${dbdir}/CONTENTS || die "pkg db write CONTENTS failed"
    if [[ -n "${D}" ]] && [[ -d "${D}" ]] ; then
        local f ff
        find "${D}/" | \
        while read f ; do
            ff=${f#${D}}
            ff=${ff//+(\/)/\/}
            [[ "${ff}" == "/" ]] && continue
            if [[ -d "${f}" ]] ; then
                echo "dir ${ff}" >> ${dbdir}/CONTENTS
            elif [[ -L "${f}" ]] ; then
                echo "sym ${ff} -> $(readlink ${f} ) $(stat -c '%Y' ${f} )" >> ${dbdir}/CONTENTS
            else
                echo "obj ${ff} $(md5sum ${f} | cut -d ' ' -f1 ) $(stat -c '%Y' ${f} )" >> ${dbdir}/CONTENTS
            fi
        done
    fi
}

ebuild_f_merge()
{
    if hasq "merge" ${RESTRICT} ; then
        ebuild_section "Skipping builtin_merge (RESTRICT)"
    elif hasq "merge" ${SKIP_FUNCTIONS} ; then
        ebuild_section "Skipping builtin_merge (SKIP_FUNCTIONS)"
    else
        ebuild_section "Starting builtin_merge"
        builtin_merge
        ebuild_section "Done builtin_merge"
    fi
}

