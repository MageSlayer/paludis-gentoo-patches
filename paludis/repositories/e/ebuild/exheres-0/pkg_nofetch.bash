#!/usr/bin/env bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2006, 2007, 2008 Ciaran McCreesh
#
# Based in part upon ebuild.sh from Portage, which is Copyright 1995-2005
# Gentoo Foundation and distributed under the terms of the GNU General
# Public License v2.
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

default_pkg_nofetch()
{
    verify_not_called_cross_phase ${FUNCNAME[0]#default_}
    [[ -z "${ARCHIVES}" ]] && return

    local f g=
    for f in ${ARCHIVES} ; do
        [[ -f "${FETCHEDDIR}/${ARCHIVES}" ]] && continue
        if [[ -z "${g}" ]] ; then
            echo "The following files could not be fetched automatically for ${PN}:"
            g=no
        fi
        echo "* ${f}"
    done
}

pkg_nofetch()
{
    default "$@"
}

exheres_internal_nofetch()
{
    local old_sandbox_write="${SANDBOX_WRITE}"
    if [[ -z "${PALUDIS_DO_NOTHING_SANDBOXY}" ]]; then
        SANDBOX_WRITE="${SANDBOX_WRITE+${SANDBOX_WRITE}:}${FETCHEDDIR}"
        esandbox check >/dev/null 2>&1 && esandbox allow "${FETCHEDDIR}"
    fi

    if hasq "nofetch" ${SKIP_FUNCTIONS} ; then
        ebuild_section "Skipping pkg_nofetch (SKIP_FUNCTIONS)"
    else
        ebuild_section "Starting pkg_nofetch"
        pkg_nofetch
        ebuild_section "Done pkg_nofetch"
    fi

    if [[ -z "${PALUDIS_DO_NOTHING_SANDBOXY}" ]]; then
        SANDBOX_WRITE="${old_sandbox_write}"
        esandbox check >/dev/null 2>&1 && esandbox disallow "${FETCHEDDIR}"
    fi
    true
}
