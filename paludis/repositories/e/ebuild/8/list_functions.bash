#!/usr/bin/env bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2006, 2009, 2012 Ciaran McCreesh
# Copyright (c) 2021-2022 Mihai Moldovan
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

ebuild_load_module --older list_functions

has()
{
    local x= want="$1"
    shift
    for x in $@ ; do
        [[ "${x}" == "${want}" ]] && return 0
    done
    return 1
}

hasq()
{
    die "hasq is banned in EAPI 8"
}

hasv()
{
    die "hasv is banned in EAPI 8"
}

in_iuse()
{
    has "${!PALUDIS_EBUILD_PHASE_VAR}" metadata init \
        && die "in_iuse must not be called in global scope"

    local i=( ${IUSE_EFFECTIVE} )
    has ${1} "${i[@]#[+-]}"
}

use()
{
    if [[ "${1:0:1}" == "!" ]] ; then
        ! has "${1#!}" "${USE}"
    else
        has "${1}" "${USE}"
    fi
}

useq()
{
    die "useq is banned in EAPI 8"
}

usev()
{
    typeset -i max_args='1'
    typeset args_desc='argument'
    if [[ -n "${PALUDIS_USEV_OUTPUT_ARG}" ]]; then
        ((++max_args))
        args_desc='arguments'
    fi

    if [[ "${#}" -gt "${max_args}" ]]; then
        die "More than ${max_args} ${args_desc} passed to usev"
    fi

    if use "${1}" ; then
        if [[ -n "${2}" ]]; then
            echo "${2}"
        else
            echo "${1#!}"
        fi
        return 0
    else
        return 1
    fi
}
