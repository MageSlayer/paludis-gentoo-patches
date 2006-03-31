#!/bin/bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

src_test()
{
    if [[ -f Makefile ]] ; then
        echo "Makefile found, looking for potential test targets"
        if emake -j1 -n check ; then
            echo "Found check target"
            emake -j1 check || die "make check failed"
        elif emake -j1 -n test ; then
            echo "Found test target"
            emake -j1 test || die "make test failed"
        else
            echo "No check or test target, skipping tests"
        fi
    else
        echo "No Makefile, skipping tests"
    fi
}

ebuild_f_test()
{
    if hasq "test" ${RESTRICT} ; then
        ebuild_section "Skipping src_test (RESTRICT)"
    elif hasq "test" ${SKIP_FUNCTIONS} ; then
        ebuild_section "Skipping src_test (SKIP_FUNCTIONS)"
    else
        ebuild_section "Starting src_test"
        src_test
        ebuild_section "Done src_test"
    fi
}

