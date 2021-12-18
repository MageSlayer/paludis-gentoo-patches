#!/usr/bin/env bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2006, 2007, 2009 Ciaran McCreesh
# Copyright (c) 2008, 2015 David Leverton
# Copyright (c) 2021 Mihai Moldovan
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
    if ! declare -p PATCHES >/dev/null 2>&1 ; then
        :
    elif declare -p PATCHES | grep -q '^declare -a ' ; then
        [[ ${#PATCHES[@]} -gt 0 ]] && eapply -- "${PATCHES[@]}"
    else
        [[ -n ${PATCHES} ]] && eapply -- ${PATCHES}
    fi
    eapply_user
}

# It would be great to not just copy the following two functions without
# actually modifying their code, but sadly, this doesn't seem to be possible,
# since ebuild.bash will just error out due to not having the ebuild_f_
# function defined.
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

        ebuild_section "Starting src_prepare"
        src_prepare
        ebuild_section "Done src_prepare"

        if [[ $(type -t post_src_prepare ) == "function" ]] ; then
            ebuild_section "Starting post_src_prepare"
            post_src_prepare
            ebuild_section "Done post_src_prepare"
        fi
    fi
}
