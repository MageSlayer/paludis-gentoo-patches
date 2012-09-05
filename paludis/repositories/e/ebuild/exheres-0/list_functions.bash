#!/usr/bin/env bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2006, 2007, 2008, 2011 Ciaran McCreesh
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

use()
{
    die "Function 'use' banned in this EAPI"
}

usev()
{
    die "Function 'usev' banned in this EAPI"
}

useq()
{
    die "Function 'useq' banned in this EAPI"
}

optionfmt()
{
    local opt="${1#!}"
    echo ${opt#*([^:]):}
}

option()
{
    [[ "${#@}" -gt 3 ]] && die "$0 takes at most three arguments"
    optionq "${1}"
    local r=$?
    if [[ ${r} -eq 0 ]] ; then
        [[ -n "${2}" ]] && echo "${2}"
    else
        [[ -n "${3}" ]] && echo "${3}"
    fi
    return ${r}
}

optionv()
{
    option "${1}" "$(optionfmt "${1}")"
}

optionq()
{
    [[ "${#@}" -ne 1 ]] && die "$0 should take exactly one arg"
    if [[ "${1:0:1}" == "!" ]] ; then
        local r=$(paludis_pipe_command OPTIONQ "$EAPI" "${1#!}" )
        return $((! ${r%%;*}))
    else
        local r=$(paludis_pipe_command OPTIONQ "$EAPI" "$1" )
        return ${r%%;*}
    fi
}

has()
{
    hasq "${@}"
}

hasv()
{
    if hasq "${@}" ; then
        echo "${1}"
        return 0
    else
        return 1
    fi
}

hasq()
{
    local x= want="$1"
    shift
    for x in $@ ; do
        [[ "${x}" == "${want}" ]] && return 0
    done
    return 1
}

expecting_tests()
{
    local a r
    case "${1}" in
        --any|--recommended|--expensive) a="${1}"; shift ;;
    esac
    r=$(paludis_pipe_command EXPECTING_TESTS "$EAPI" "${a:---any}" )
    [[ "${#@}" -gt 2 ]] && die "$0 takes at most three arguments"
    if [[ ${r%%;*} -eq 0 ]] ; then
        [[ -n "${1}" ]] && echo "${1}"
    else
        [[ -n "${2}" ]] && echo "${2}"
    fi
    return ${r%%;*}
}

