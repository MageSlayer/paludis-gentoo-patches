#!/bin/bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2006, 2007 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

export EBUILD_KILL_PID=$$
declare -r EBUILD_KILL_PID

alias die='diefunc "$FUNCNAME" "$LINENO"'
alias assert='_pipestatus="${PIPESTATUS[*]}"; [[ -z "${_pipestatus//[ 0]/}" ]] || diefunc "$FUNCNAME" "$LINENO" "$_pipestatus"'
trap 'echo "die trap: exiting with error." 1>&2 ; exit 250' SIGUSR1

diefunc()
{
    local func="$1" line="$2"
    shift 2
    echo 1>&2
    echo "!!! ERROR in ${CATEGORY:-?}/${PF:-?}:" 1>&2
    echo "!!! In ${func:-?} at line ${line:-?}" 1>&2
    echo "!!! ${*:-(no message provided)}" 1>&2
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


