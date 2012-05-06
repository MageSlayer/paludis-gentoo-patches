#!/usr/bin/env bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2006, 2007, 2011 Ciaran McCreesh
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

src_test()
{
    if [[ -f Makefile ]] || [[ -f GNUmakefile ]] || [[ -f makefile ]] ; then
        echo "Makefile found, looking for potential test targets"
        if make -j1 -n check ; then
            echo "Found check target"
            emake check || die "make check failed"
        elif make -j1 -n test ; then
            echo "Found test target"
            emake test || die "make test failed"
        else
            echo "No check or test target, skipping tests"
        fi
    else
        echo "No Makefile, skipping tests"
    fi
}

ebuild_f_test()
{
    local old_sandbox_predict="${SANDBOX_PREDICT}"
    [[ -z "${PALUDIS_DO_NOTHING_SANDBOXY}" ]] && SANDBOX_PREDICT="${SANDBOX_PREDICT+${SANDBOX_PREDICT}:}/"

    local save_PALUDIS_EXTRA_DIE_MESSAGE="${PALUDIS_EXTRA_DIE_MESSAGE}"
    export PALUDIS_EXTRA_DIE_MESSAGE="
!!! This package failed inside the test phase. You should read
!!!    http://paludis.exherbo.org/faq/stricter.html#testfailures
!!! for more information on packages with test phase failures.
"

    if [[ -d "${S}" ]] ; then
        cd "${S}" || die "cd to \${S} (\"${S}\") failed"
    elif [[ -d "${WORKDIR}" ]] ; then
        cd "${WORKDIR}" || die "cd to \${WORKDIR} (\"${WORKDIR}\") failed"
    fi

    if hasq "test" ${SKIP_FUNCTIONS} ; then
        ebuild_section "Skipping src_test (SKIP_FUNCTIONS)"
    else
        if [[ $(type -t pre_src_test ) == "function" ]] ; then
            ebuild_section "Starting pre_src_test"
            pre_src_test
            ebuild_section "Done pre_src_test"
        fi

        ebuild_section "Starting src_test"
        src_test
        ebuild_section "Done src_test"

        if [[ $(type -t post_src_test ) == "function" ]] ; then
            ebuild_section "Starting post_src_test"
            post_src_test
            ebuild_section "Done post_src_test"
        fi
    fi

    export PALUDIS_EXTRA_DIE_MESSAGE="${save_PALUDIS_EXTRA_DIE_MESSAGE}"

    [[ -z "${PALUDIS_DO_NOTHING_SANDBOXY}" ]] && SANDBOX_PREDICT="${old_sandbox_predict}"
    true
}

