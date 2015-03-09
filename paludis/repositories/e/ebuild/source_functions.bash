#!/usr/bin/env bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2007, 2008 David Leverton
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

ebuild_need_extglob()
{
    eval "_ebuild_need_extglob_$(declare -f ${1})"
    eval "
        ${1}()
        {
            eval \"
                shopt -s extglob
                _ebuild_need_extglob_${1} \\\"\\\${@}\\\"
                eval \\\"\$(shopt -p extglob); return \\\${?}\\\"
            \"
        }"
}

ebuild_safe_source()
{
    set -- "${@}" '[^a-zA-Z_]*' '*[^a-zA-Z0-9_]*' \
        EUID PPID UID FUNCNAME GROUPS SHELLOPTS BASHOPTS BASHPID IFS PWD \
        'BASH_@(ARGC|ARGV|LINENO|SOURCE|VERSINFO|REMATCH)' \
        'BASH_COMPLETION?(_DIR)' 'bash+([0-9])?([a-z])' \
        EBUILD_KILL_PID PALUDIS_LOADSAVEENV_DIR PALUDIS_DO_NOTHING_SANDBOXY SANDBOX_ACTIVE \
        PALUDIS_IGNORE_PIVOT_ENV_FUNCTIONS PALUDIS_IGNORE_PIVOT_ENV_VARIABLES \
        PALUDIS_PIPE_COMMAND_READ_FD PALUDIS_PIPE_COMMAND_WRITE_FD

    trap DEBUG
    set -T
    shopt -s extdebug
    trap "[[ \${BASH_COMMAND%%=*} == ?(*[[:space:]])!($(IFS='|'; [[ ${1} == --rewrite-for-declare ]] && shift; shift; echo "${*}")) ||
              \${BASH_COMMAND%%[[:space:]]*} != @(*=*|export|declare) ]]" DEBUG

    if [[ ${1} == --rewrite-for-declare ]]; then
        ( source "${2}" && set >"${2}" && print_exports >>"${2}" ) && source "${2}"
    else
        source "${1}"
    fi
    eval "trap DEBUG; shopt -u extdebug; set +T; return ${?}"
}
ebuild_need_extglob ebuild_safe_source

ebuild_verify_not_changed_from_global_scope()
{
    local v vv_orig
    for v in "$@" ; do
        vv_orig=PALUDIS_SAVE_GLOBAL_SCOPE_$v
        vv_orig=${!vv_orig}
        [[ "${vv_orig}" == "$(declare -p "${v}" 2>/dev/null)" ]] || die "Variable $v must be set to an invariant value in global scope"
    done
}

