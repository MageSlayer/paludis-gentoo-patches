#!/usr/bin/env bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2008 David Leverton
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
    [[ -z "${A}" ]] && return

    local f g=
    for f in ${A} ; do
        [[ -f "${DISTDIR}/${A}" ]] && continue
        if [[ -z "${g}" ]] ; then
            echo "The following files could not be fetched automatically for ${PN}:"
            g=no
        fi
        echo "* ${f}"
    done
}

pkg_nofetch()
{
    default_pkg_nofetch
}

ebuild_f_nofetch()
{
    local old_sandbox_write="${SANDBOX_WRITE}"
    [[ -z "${PALUDIS_DO_NOTHING_SANDBOXY}" ]] && SANDBOX_WRITE="${SANDBOX_WRITE+${SANDBOX_WRITE}:}${DISTDIR}"
    if hasq "nofetch" ${SKIP_FUNCTIONS} ; then
        ebuild_section "Skipping pkg_nofetch (SKIP_FUNCTIONS)"
    else
        ebuild_section "Starting pkg_nofetch"
        pkg_nofetch
        ebuild_section "Done pkg_nofetch"
    fi
    [[ -z "${PALUDIS_DO_NOTHING_SANDBOXY}" ]] && SANDBOX_WRITE="${old_sandbox_write}"
    true
}

