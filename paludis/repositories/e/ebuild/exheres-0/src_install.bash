#!/usr/bin/env bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2006, 2007, 2008, 2009 Ciaran McCreesh
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

default_src_install()
{
    verify_not_called_cross_phase ${FUNCNAME[0]#default_}
    ebuild_verify_not_changed_from_global_scope DEFAULT_SRC_INSTALL_PARAMS

    if [[ -f Makefile ]] || [[ -f makefile ]] || [[ -f GNUmakefile ]] ; then
        if make -j1 -n DESTDIR="${IMAGE}" "${DEFAULT_SRC_INSTALL_PARAMS[@]}" install ; then
            echo "Found a makefile, using the install target" 
            emake -j1 DESTDIR="${IMAGE}" "${DEFAULT_SRC_INSTALL_PARAMS[@]}" install
        else
            die "default emake install located a makefile but no install target"
        fi
    else
        echo "No makefile found, not using emake install"
    fi
    emagicdocs
}

src_install()
{
    default "$@"
}

exheres_internal_install()
{
    cd "${WORK}" || die "cd to \${WORK} (\"${WORK}\") failed"

    if hasq "install" ${SKIP_FUNCTIONS} ; then
        ebuild_section "Skipping src_install (SKIP_FUNCTIONS)"
    else
        ebuild_section "Starting src_install"
        src_install
        ebuild_section "Done src_install"
    fi
}
