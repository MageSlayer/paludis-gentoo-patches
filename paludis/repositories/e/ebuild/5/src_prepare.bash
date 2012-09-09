#!/usr/bin/env bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2006, 2007, 2009 Ciaran McCreesh
# Copyright (c) 2008, 2012 David Leverton
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

default_src_prepare()
{
    apply_user_patches
}

src_prepare()
{
    default_src_prepare
}

ebuild_f_prepare()
{
    if [[ -d "${S}" ]] ; then
        cd "${S}" || die "cd to \${S} (\"${S}\") failed"
    elif [[ -n "${PALUDIS_NO_S_WORKDIR_FALLBACK}" ]] ; then
        die "\${S} (\"${S}\") does not exist"
    elif [[ -d "${WORKDIR}" ]] ; then
        cd "${WORKDIR}" || die "cd to \${WORKDIR} (\"${WORKDIR}\") failed"
    fi

    if hasq "prepare" ${SKIP_FUNCTIONS} ; then
        ebuild_section "Skipping src_prepare (SKIP_FUNCTIONS)"
    else
        if [[ $(type -t pre_src_prepare ) == "function" ]] ; then
            ebuild_section "Starting pre_src_prepare"
            pre_src_prepare
            ebuild_section "Done pre_src_prepare"
        fi

        rm -f "${T}"/paludis-apply_user_patches-called

        ebuild_section "Starting src_prepare"
        src_prepare
        ebuild_section "Done src_prepare"

        [[ -f "${T}"/paludis-apply_user_patches-called ]] \
            || die "apply_user_patches must be called during src_prepare"

        if [[ $(type -t post_src_prepare ) == "function" ]] ; then
            ebuild_section "Starting post_src_prepare"
            post_src_prepare
            ebuild_section "Done post_src_prepare"
        fi
    fi
}

