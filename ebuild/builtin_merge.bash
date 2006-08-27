#!/bin/bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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
    ebuild_section "Merging to '${ROOT:-/}'..."

    shopt -q dotglob
    local olddotglob=$?
    shopt -s dotglob

    local v=$(vdb_path)
    if [[ -z "${v}" ]] ; then
        v=${ROOT}/var/db/pkg
    fi
    local dbdir="${v}/${CATEGORY}/${PF}"
    ebuild_section "Writing VDB entry to '${dbdir}'..."
    install -d "${dbdir}" || die "couldn't make pkg db directory (\"${dbdir}\")"
    install -d "${v}/".cache || die "couldn't make pkg db cache"

    local v VDB_FORMAT="paludis-2" COUNTER="0"
    for v in CATEGORY CBUILD CHOST COUNTER DEPEND DESCRIPTION EAPI \
        FEATURES HOMEPAGE INHERITED IUSE KEYWORDS LICENSE PDEPEND PF \
        PROVIDE RDEPEND SLOT SRC_URI USE CONFIG_PROTECT CONFIG_PROTECT_MASK \
        VDB_FORMAT ; do
        echo "${!v}" > "${dbdir}"/${v} || die "pkg db write ${v} failed"
    done
    for v in ASFLAGS CBUILD CC CFLAGS CHOST CTARGET CXX CXXFLAGS \
        EXTRA_ECONF EXTRA_EINSTALL EXTRA_EMAKE LDFLAGS LIBCXXFLAGS \
        REPOSITORY ; do
        [[ -z "${!v}" ]] && continue
        echo "${!v}" > "${dbdir}"/${v} || die "pkg db write ${v} failed"
    done

    if [[ -n ${PALUDIS_EBUILD_OVERRIDE_CONFIG_PROTECT} ]]; then
        CONFIG_PROTECT=${PALUDIS_EBUILD_OVERRIDE_CONFIG_PROTECT}
    fi
    if [[ -n ${PALUDIS_EBUILD_OVERRIDE_CONFIG_PROTECT_MASK} ]]; then
        CONFIG_PROTECT_MASK=${PALUDIS_EBUILD_OVERRIDE_CONFIG_PROTECT_MASK}
    fi

    export CONFIG_PROTECT="${CONFIG_PROTECT}"
    export CONFIG_PROTECT_MASK="${CONFIG_PROTECT_MASK}"

    [[ -f "${EBUILD}" ]] && cp "${EBUILD}" ${dbdir}/

    local reinstall=
    if [[ -f "${dbdir}/CONTENTS" ]] ; then
        mv "${dbdir}/CONTENTS" "${dbdir}/OLDCONTENTS" || die "save contents failed"
        reinstall="yes"
    fi

    ( set ; export -p | sed 's:^declare -rx:declare -x:' ) | bzip2 > ${dbdir}/environment.bz2
    > ${dbdir}/CONTENTS

    if [[ -n "${D}" ]] && [[ -d "${D}" ]] ; then
        install -d "${ROOT%/}/" || die "couldn't make \${ROOT} (\"${ROOT}\")"
        if [[ -d "${D}" ]] ; then
            merge "${D%/}/" "${ROOT%/}/" "${dbdir}/CONTENTS" \
                || die "merge failed"
        fi
    fi

    if ! /bin/sh -c 'echo Good, our shell is still usable' ; then
        echo "Looks like our shell broke. Trying an ldconfig to fix it..."
        ldconfig -r ${ROOT}
    fi

    if [[ -n "${reinstall}" ]] ; then
        unmerge "${ROOT%/}/" "${dbdir}/OLDCONTENTS" \
            || die "unmerge failed"

        if ! /bin/sh -c 'echo Good, our shell is still usable' ; then
            echo "Looks like our shell broke. Trying an ldconfig to fix it..."
            ldconfig -r ${ROOT}
        fi

        rm -f "${dbdir}/OLDCONTENTS"
    fi

    [[ $olddotglob != 0 ]] && shopt -u dotglob
    shopt -q dotglob
    [[ $olddotglob == $? ]] || ebuild_notice "warning" "shopt dotglob restore failed"
}

ebuild_f_merge()
{
    local old_sandbox_write="${SANDBOX_WRITE}"
    SANDBOX_WRITE="${SANDBOX_WRITE+${SANDBOX_WRITE}:}${ROOT%/}/"
    local old_sandbox_on="${SANDBOX_ON}"
    [[ "$(canonicalise ${ROOT} )" != "/" ]] || SANDBOX_ON=0

    if hasq "merge" ${RESTRICT} ; then
        ebuild_section "Skipping builtin_merge (RESTRICT)"
    elif hasq "merge" ${SKIP_FUNCTIONS} ; then
        ebuild_section "Skipping builtin_merge (SKIP_FUNCTIONS)"
    else
        ebuild_section "Starting builtin_merge"
        builtin_merge
        ebuild_section "Done builtin_merge"
    fi

    SANDBOX_WRITE="${old_sandbox_write}"
    SANDBOX_ON="${old_sandbox_on}"
}

