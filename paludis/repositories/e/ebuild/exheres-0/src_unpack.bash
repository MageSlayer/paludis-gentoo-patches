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

default_src_unpack()
{
    verify_not_called_cross_phase ${FUNCNAME[0]#default_}
    [[ -n "${ARCHIVES}" ]] && unpack --if-compressed ${ARCHIVES}
}

src_unpack()
{
    default "$@"
}

exheres_internal_unpack()
{
    cd "${WORKBASE}" || die "cd to \${WORKBASE} (\"${WORKBASE}\") failed"

    if hasq "unpack" ${SKIP_FUNCTIONS} ; then
        ebuild_section "Skipping src_unpack (SKIP_FUNCTIONS)"
    else
        ebuild_section "Starting src_unpack"
        src_unpack
        ebuild_section "Done src_unpack"
    fi
}
