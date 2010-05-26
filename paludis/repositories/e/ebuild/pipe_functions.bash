#!/usr/bin/env bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2007, 2008, 2009, 2010 Ciaran McCreesh
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

paludis_pipe_command()
{
    [[ -n "${PALUDIS_SKIP_PIPE_COMMAND_CHECK}" ]] && return

    if [[ -z "${PALUDIS_PIPE_COMMAND_WRITE_FD}" ]]; then
        type die &>/dev/null && eval die "\"PALUDIS_PIPE_COMMAND_WRITE_FD unset\""
        echo "PALUDIS_PIPE_COMMAND_WRITE_FD unset" 1>&2
        if [[ -n ${EBUILD_KILL_PID} ]]; then
            echo "paludis_pipe_command: making ebuild PID ${EBUILD_KILL_PID} exit with error" 1>&2
            kill -s SIGUSR1 "${EBUILD_KILL_PID}"
        fi
        exit 123
    fi
    if [[ -z "${PALUDIS_PIPE_COMMAND_READ_FD}" ]]; then
        type die &>/dev/null && eval die "\"PALUDIS_PIPE_COMMAND_READ_FD unset\""
        echo "PALUDIS_PIPE_COMMAND_READ_FD unset" 1>&2
        if [[ -n ${EBUILD_KILL_PID} ]]; then
            echo "paludis_pipe_command: making ebuild PID ${EBUILD_KILL_PID} exit with error" 1>&2
            kill -s SIGUSR1 "${EBUILD_KILL_PID}"
        fi
        exit 124
    fi

    local r r1 rest a
    r="$(for a in "$@" ; do echo -n "${a}${PALUDIS_PIPE_COMMAND_DELIM:- }" ; done | {
        if ! locked_pipe_command "${PALUDIS_PIPE_COMMAND_WRITE_FD}" "${PALUDIS_PIPE_COMMAND_READ_FD}" ; then
            type die &>/dev/null && eval die "\"locked_pipe_command failed\""
            echo "locked_pipe_command failed" 1>&2
            if [[ -n ${EBUILD_KILL_PID} ]]; then
                echo "paludis_pipe_command: making ebuild PID ${EBUILD_KILL_PID} exit with error" 1>&2
                kill -s SIGUSR1 "${EBUILD_KILL_PID}"
            fi
            exit 125
        fi
    })"

    r1="${r:0:1}"
    rest="${r:1}"
    if [[ "${r1}" != "O" ]] ; then
        type die &>/dev/null && eval die "\"paludis_pipe_command returned error '\${r1}' with text '\${rest}'\""
        echo "paludis_pipe_command returned error '${r1}' with text '${rest}'" 1>&2
        if [[ -n ${EBUILD_KILL_PID} ]]; then
            echo "paludis_pipe_command: making ebuild PID ${EBUILD_KILL_PID} exit with error" 1>&2
            kill -s SIGUSR1 "${EBUILD_KILL_PID}"
        fi
        exit 126
    fi

    echo "$rest"
}

paludis_rewrite_var()
{
    [[ "${#@}" -ne 3 ]] && die "$0 should take exactly three args"
    local r="$(paludis_pipe_command REWRITE_VAR "$EAPI" "$1" "$2" "$3" )"
    echo "${r#*;}"
    return ${r%%;*}
}

