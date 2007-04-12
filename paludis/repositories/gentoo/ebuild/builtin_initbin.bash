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

builtin_initbin()
{
    export ROOT="${ROOT//+(\/)//}"

    local a
    for a in P PV PR PN PVR PF CATEGORY PORTDIR \
        PKGDIR KV PALUDIS_TMPDIR PALUDIS_EBUILD_LOG_LEVEL PALUDIS_EBUILD_DIR \
        CHOST PALUDIS_COMMAND ROOT PALUDIS_LOADSAVEENV_DIR ; do
        [[ -z "${!a}" ]] && die "\$${a} unset or empty"
    done

    for a in PKGDIR ; do
        [[ -d "${!a}" ]] || die "\$${a} (\"${!a}\") not a directory"
    done

    if [[ -e "${PALUDIS_TMPDIR}/${CATEGORY}/${PF}" ]] ; then
        if type -p chflags &>/dev/null; then
            chflags -R 0 "${PALUDIS_TMPDIR}/${CATEGORY}/${PF}" || die "Couldn't remove flags from workdir"
        fi
        rm -fr "${PALUDIS_TMPDIR}/${CATEGORY}/${PF}" || die "Couldn't remove previous work"
    fi

    export WORKDIR="${PALUDIS_TMPDIR}/${CATEGORY}/${PF}/work"
    mkdir -p "${WORKDIR}" || die "Couldn't create \$WORKDIR (\"${WORKDIR}\")"

    export T="${PALUDIS_TMPDIR}/${CATEGORY}/${PF}/temp/"
    mkdir -p "${T}" || die "Couldn't create \$T (\"${T}\")"
    export HOME="${T}"

    export D="${PALUDIS_TMPDIR}/${CATEGORY}/${PF}/image/"
    export D="${D//+(\/)//}"
    mkdir -p "${D}" || die "Couldn't create \$D (\"${D}\")"

    export IMAGE="${D}"

    export S="${WORKDIR}/${P}"

    [[ -n "${B}" ]] && unpack --binary --only .paludis-binpkg-environment ${B}

    [[ -f "${IMAGE}/.paludis-binpkg-environment" ]] || \
        die "No saved environment in binary tarball"

    local save_PALUDIS_EXTRA_DIE_MESSAGE="${PALUDIS_EXTRA_DIE_MESSAGE}"

    echo ebuild_scrub_environment "${IMAGE}/.paludis-binpkg-environment" 1>&2
    ebuild_scrub_environment "${IMAGE}/.paludis-binpkg-environment" \
        || die "Can't load saved environment for cleaning"

    echo source "${IMAGE}/.paludis-binpkg-environment" 1>&2
    source "${IMAGE}/.paludis-binpkg-environment" \
        || die "Can't load saved environment"

    export PALUDIS_EXTRA_DIE_MESSAGE="${save_PALUDIS_EXTRA_DIE_MESSAGE}"

    echo rm "${IMAGE}/.paludis-binpkg-environment" 1>&2
    rm "${IMAGE}/.paludis-binpkg-environment"
}

ebuild_f_initbin()
{
    if hasq "initbin" ${RESTRICT} ; then
        ebuild_section "Skipping builtin_initbin (RESTRICT)"
    elif hasq "init" ${SKIP_FUNCTIONS} ; then
        ebuild_section "Skipping builtin_initbin (SKIP_FUNCTIONS)"
    else
        ebuild_section "Starting builtin_initbin"
        builtin_initbin
        ebuild_section "Done builtin_initbin"
    fi
}


