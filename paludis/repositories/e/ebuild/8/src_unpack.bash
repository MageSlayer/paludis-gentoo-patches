#!/usr/bin/env bash
# vim: set sw=4 sts=4 et :

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

ebuild_load_module --older src_unpack

ebuild_f_unpack()
{
    cd ${WORKDIR} || die "cd to \${WORKDIR} (\"${WORKDIR}\") failed"

    if has "unpack" ${SKIP_FUNCTIONS} ; then
        ebuild_section "Skipping src_unpack (SKIP_FUNCTIONS)"
    else
        if [[ $(type -t pre_src_unpack ) == "function" ]] ; then
            ebuild_section "Starting pre_src_unpack"
            pre_src_unpack
            ebuild_section "Done pre_src_unpack"
        fi

        ebuild_section "Starting src_unpack"
        src_unpack
        ebuild_section "Done src_unpack"

        if [[ $(type -t post_src_unpack ) == "function" ]] ; then
            ebuild_section "Starting post_src_unpack"
            post_src_unpack
            ebuild_section "Done post_src_unpack"
        fi
    fi
}
