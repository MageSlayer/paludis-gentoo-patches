#!/usr/bin/env bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2006, 2007, 2009 Ciaran McCreesh
# Copyright (c) 2008 David Leverton
# Copyright (c) 2022 Mihai Moldovan
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

ebuild_load_module --older src_configure

ebuild_f_configure()
{
    if [[ -d "${S}" ]] ; then
        cd "${S}" || die "cd to \${S} (\"${S}\") failed"
    elif [[ -n "${PALUDIS_NO_S_WORKDIR_FALLBACK}" ]] ; then
        die "\${S} (\"${S}\") does not exist"
    elif [[ -d "${WORKDIR}" ]] ; then
        cd "${WORKDIR}" || die "cd to \${WORKDIR} (\"${WORKDIR}\") failed"
    fi

    if has "configure" ${SKIP_FUNCTIONS} ; then
        ebuild_section "Skipping src_configure (SKIP_FUNCTIONS)"
    else
        if [[ $(type -t pre_src_configure ) == "function" ]] ; then
            ebuild_section "Starting pre_src_configure"
            pre_src_configure
            ebuild_section "Done pre_src_configure"
        fi

        ebuild_section "Starting src_configure"
        src_configure
        ebuild_section "Done src_configure"

        if [[ $(type -t post_src_configure ) == "function" ]] ; then
            ebuild_section "Starting post_src_configure"
            post_src_configure
            ebuild_section "Done post_src_configure"
        fi
    fi
}
