#!/usr/bin/env bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2006, 2007, 2009 Ciaran McCreesh
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

shopt -s expand_aliases

_paludis_pipestatus=
alias die='diefunc "${FUNCNAME:-$0}" "$LINENO"'
alias assert='_paludis_pipestatus="${PIPESTATUS[*]}"; [[ -z "${_paludis_pipestatus//[ 0]/}" ]] || diefunc "${FUNCNAME:-$0}" "$LINENO"'
# paludis_die_or_error is only for use in scripts
alias paludis_die_or_error='paludis_die_or_error_func "$0" "$LINENO"'
# paludis_die_unless_nonfatal and paludis_assert_unless_nonfatal are only for use in shell functions
alias paludis_die_unless_nonfatal='paludis_die_unless_nonfatal_func "$FUNCNAME" "$LINENO"'
alias paludis_assert_unless_nonfatal='_paludis_pipestatus="${PIPESTATUS[*]}"; [[ -z "${_paludis_pipestatus//[ 0]/}" ]] || paludis_die_unless_nonfatal_func "$FUNCNAME" "$LINENO"'

trap 'echo "die trap: exiting with error." 1>&2 ; exit 250' SIGUSR1

diefunc()
{
    local func="$1" line="$2" nonfatal=
    shift 2

    if [[ -n ${PALUDIS_DIE_SUPPORTS_DASH_N} && $1 == -n && -n ${PALUDIS_FAILURE_IS_NONFATAL} ]] ; then
        shift
        nonfatal=yes
    fi

    local message="$*"
    [[ -n ${_paludis_pipestatus//[ 0]/} ]] && message="${_paludis_pipestatus} ${message}"

    if [[ -n ${nonfatal} ]] ; then
        echo "${func}: ${message}" >&2
        return 247
    fi

    echo 1>&2
    echo "!!! ERROR in ${CATEGORY:-?}/${!PALUDIS_NAME_VERSION_REVISION_VAR:-?}::${REPOSITORY:-?}:" 1>&2
    echo "!!! In ${func:-?} at line ${line:-?}" 1>&2
    echo "!!! ${message:-(no message provided)}" 1>&2
    echo 1>&2

    echo "!!! Call stack:" 1>&2
    for (( n = 1 ; n < ${#FUNCNAME[@]} ; ++n )) ; do
        funcname=${FUNCNAME[${n}]}
        sourcefile=${BASH_SOURCE[${n}]}
        lineno=${BASH_LINENO[$(( n - 1 ))]}
        echo "!!!    * ${funcname} (${sourcefile}:${lineno})" 1>&2
    done
    echo 1>&2

    if [[ -n "${PALUDIS_EXTRA_DIE_MESSAGE}" ]] ; then
        echo "${PALUDIS_EXTRA_DIE_MESSAGE}" 1>&2
        echo 1>&2
    fi

    echo "diefunc: making ebuild PID ${EBUILD_KILL_PID} exit with error" 1>&2
    kill -s SIGUSR1 "${EBUILD_KILL_PID}"
    exit 249
}

paludis_die_or_error_func()
{
    if [[ -n ${PALUDIS_FAILURE_IS_FATAL} ]]; then
        diefunc "$@"
    else
        local func=${1}
        shift 2
        echo "${func}: $*" >&2
        exit 247
    fi
}

paludis_die_unless_nonfatal_func()
{
    if [[ -z ${PALUDIS_FAILURE_IS_NONFATAL} ]]; then
        diefunc "$@"
    else
        local func=${1}
        shift 2
        echo "${func}: $*" >&2
        return 247
    fi
}
