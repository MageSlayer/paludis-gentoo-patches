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

pkg_init()
{
    local a
    for a in P PV PR PN PVR PF CATEGORY FILESDIR ECLASSDIR PORTDIR \
        DISTDIR KV PALUDIS_TMPDIR PALUDIS_EBUILD_LOG_LEVEL PALUDIS_EBUILD_DIR ; do
        [[ -z "${!a}" ]] && die "\$${a} unset or empty"
    done

    for a in FILESDIR ECLASSDIR PORTDIR DISTDIR ; do
        [[ -d "${!a}" ]] || die "\$${a} (\"${!a}\") not a directory"
    done

    export WORKDIR="${PALUDIS_TMPDIR}/${CATEGORY}/${PF}/work"
    if [[ -f "${WORKDIR}" ]] ; then
        rm -fr "${WORKDIR}" || die "Couldn't clean out \$WORKDIR (\"${WORKDIR}\")"
    fi
    mkdir -p "${WORKDIR}" || die "Couldn't create \$WORKDIR (\"${WORKDIR}\")"

    export T="${PALUDIS_TMPDIR}/${CATEGORY}/${PF}/temp"
    mkdir -p "${T}" || die "Couldn't create \$T (\"${T}\")"

    export D="${PALUDIS_TMPDIR}/${CATEGORY}/${PF}/image"
    mkdir -p "${D}" || die "Couldn't create \$D (\"${D}\")"

    export S="${WORKDIR}/${P}"

    export PATH="${PALUDIS_EBUILD_DIR}/utils:${PATH}"
}

ebuild_f_init()
{
    ebuild_section "Starting pkg_init"
    pkg_init
    ebuild_section "Done pkg_init"
}

