#!/bin/bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
#
# Based in part upon ebuild.sh from Portage, which is Copyright 1995-2005
# Gentoo Foundation and distributed under the terms of the GNU General
# Public License v2.
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

builtin_unpack_bin()
{
    unpack --binary ${B}
    local ebuild_dir="${IMAGE}/tmp/paludis-ebin/${SRC_REPOSITORY}/${CATEGORY}/${PF}/"
    [[ -d "${ebuild_dir}" ]] || die "ebuild_dir not a directory. Invalid binary package?"
    mv "${ebuild_dir}/"* "${WORKDIR}"/
    rm -fr "${IMAGE}/tmp/paludis-ebin"
    export PALUDIS_LOAD_ENVIRONMENT="${WORKDIR}/environment.bz2"
    ebuild_load_ebuild "${WORKDIR}/${PF}.ebuild"
}

ebuild_f_unpack_bin()
{
    cd ${WORKDIR} || die "cd to \${WORKDIR} (\"${WORKDIR}\") failed"

    if hasq "unpack_bin" ${RESTRICT} ; then
        ebuild_section "Skipping builtin_unpack_bin (RESTRICT)"
    elif hasq "unpack_bin" ${SKIP_FUNCTIONS} ; then
        ebuild_section "Skipping builtin_unpack_bin (SKIP_FUNCTIONS)"
    else
        ebuild_section "Starting builtin_unpack_bin"
        builtin_unpack_bin
        ebuild_section "Done builtin_unpack_bin"
    fi
}


