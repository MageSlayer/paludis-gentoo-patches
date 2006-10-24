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

src_compile()
{
    [[ -x ./configure ]] && econf
    if [[ -f Makefile ]] || [[ -f makefile ]] || [[ -f GNUmakefile ]] ; then
        emake || die "emake failed"
    fi
}

ebuild_f_compile()
{
    if [[ -d "${S}" ]]; then
        cd "${S}" || die "cd to \${S} (\"${S}\") failed"
    fi

    if hasq "compile" ${RESTRICT} ; then
        ebuild_section "Skipping src_compile (RESTRICT)"
    elif hasq "compile" ${SKIP_FUNCTIONS} ; then
        ebuild_section "Skipping src_compile (SKIP_FUNCTIONS)"
    else
        ebuild_section "Starting src_compile"
        src_compile
        ebuild_section "Done src_compile"
    fi
}

