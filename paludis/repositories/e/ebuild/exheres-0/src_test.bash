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

default_src_test()
{
    verify_not_called_cross_phase ${FUNCNAME[0]#default_}
    ebuild_verify_not_changed_from_global_scope DEFAULT_SRC_TEST_PARAMS

    if [[ -f Makefile ]] || [[ -f GNUmakefile ]] || [[ -f makefile ]] ; then
        echo "Makefile found, looking for potential test targets"
        if make -j1 -n "${DEFAULT_SRC_TEST_PARAMS[@]}" check ;  then
            echo "Found check target"
            emake "${DEFAULT_SRC_TEST_PARAMS[@]}" check
        elif make -j1 -n "${DEFAULT_SRC_TEST_PARAMS[@]}" test ; then
            echo "Found test target"
            emake "${DEFAULT_SRC_TEST_PARAMS[@]}" test
        else
            echo "No check or test target, skipping tests"
        fi
    else
        echo "No Makefile, skipping tests"
    fi
}

src_test()
{
    default "$@"
}

exheres_internal_test()
{
    local save_PALUDIS_EXTRA_DIE_MESSAGE="${PALUDIS_EXTRA_DIE_MESSAGE}"

    if [[ -d "${WORK}" ]] ; then
        cd "${WORK}" || die "cd to \${WORK} (\"${WORK}\") failed"
    elif [[ -d "${WORKBASE}" ]] ; then
        cd "${WORKBASE}" || die "cd to \${WORKBASE} (\"${WORKBASE}\") failed"
    fi

    if hasq "test" ${SKIP_FUNCTIONS} ; then
        ebuild_section "Skipping src_test (SKIP_FUNCTIONS)"
    else
        ebuild_section "Starting src_test"

        if ! exhost --is-native -q ; then
            echo "cross compiled host, skipping test"
            return
        fi

        src_test
        ebuild_section "Done src_test"
    fi

    export PALUDIS_EXTRA_DIE_MESSAGE="${save_PALUDIS_EXTRA_DIE_MESSAGE}"
    true
}
