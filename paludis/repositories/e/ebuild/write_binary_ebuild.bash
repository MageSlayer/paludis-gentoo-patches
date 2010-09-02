#!/usr/bin/env bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2006, 2007, 2008, 2010 Ciaran McCreesh
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

unalias -a
set +C
unset GZIP BZIP BZIP2 CDPATH GREP_OPTIONS GREP_COLOR GLOBIGNORE
eval unset LANG ${!LC_*}

shopt -s expand_aliases
shopt -s extglob

if [[ -n "${PALUDIS_EBUILD_DIR_FALLBACK}" ]] ; then
    export PATH="${PALUDIS_EBUILD_DIR_FALLBACK}/utils:${PATH}"
fi
export PATH="${PALUDIS_EBUILD_DIR}/utils:${PATH}"
for p in ${PALUDIS_UTILITY_PATH_SUFFIXES} ; do
    export PATH="${PALUDIS_EBUILD_DIR}/utils/${p}:${PATH}"
done

# Force a few more things into PATH, since some users have crazy setups.
# See ticket:374.
export PATH="${PATH}:/bin:/sbin:/usr/bin:/usr/sbin"

EBUILD_MODULES_DIR=$(canonicalise $(dirname $0 ) )
if ! [[ -d ${EBUILD_MODULES_DIR} ]] ; then
    echo "${EBUILD_MODULES_DIR} is not a directory" 1>&2
    exit 123
fi

[[ -z "${PALUDIS_EBUILD_MODULE_SUFFIXES}" ]] && PALUDIS_EBUILD_MODULE_SUFFIXES=0
for p in ${PALUDIS_EBUILD_MODULE_SUFFIXES}; do
    EBUILD_MODULES_DIRS="${EBUILD_MODULES_DIRS} ${EBUILD_MODULES_DIR}/${p}"
done
for p in ${PALUDIS_EXTRA_EBUILD_MODULES_DIRS} ; do
    EBUILD_MODULES_DIRS="${EBUILD_MODULES_DIRS} ${p}"
done
EBUILD_MODULES_DIRS="${EBUILD_MODULES_DIRS} ${EBUILD_MODULES_DIR}"

export PALUDIS_EBUILD_MODULES_DIR="${EBUILD_MODULES_DIR}"

export EBUILD_KILL_PID=$$
declare -r EBUILD_KILL_PID

ebuild_load_module()
{
    local older= t= d= save_excl= excl_v=
    if [[ "${1}" == "--older" ]] ; then
        shift
        older=true
        excl_v="EBUILD_MODULES_DIRS_EXCLUDE_${1}"
        save_excl="${!excl_v}"
    fi

    for d in ${EBUILD_MODULES_DIRS}; do
        local dx= x=
        if [[ -n "${older}" ]] ; then
            for dx in ${!excl_v} ; do
                [[ "${dx}" == "${d}" ]] && x=true
            done
        fi
        [[ -n "${x}" ]] && continue

        [[ -n "${older}" ]] && export "${excl_v}"="${!excl_v} ${d}"
        if [[ -f "${d}/${1}.bash" ]]; then
            if ! source "${d}/${1}.bash"; then
                type die &>/dev/null && eval die "\"Error loading module \${1}\""
                echo "Error loading module ${1}" 1>&2
                exit 124
            fi
            return
        else
            t="${t:+${t}, }${d}"
        fi
    done

    [[ -n "${older}" ]] && export "${excl_v}"="${save_excl}"

    type die &>/dev/null && eval die "\"Couldn't find module \${1} (looked in \${t})\""
    echo "Couldn't find module ${1} (looked in ${t})" 1>&2
    exit 125
}

ebuild_load_module die_functions
ebuild_load_module pipe_functions
ebuild_load_module output_functions
ebuild_load_module echo_functions
ebuild_load_module source_functions
ebuild_load_module binary_functions

export PALUDIS_HOME="$(canonicalise ${PALUDIS_HOME:-${HOME}} )"

main()
{
    local ebuildfile="${1}" bindistfile="${2}" envfile="${3}" imagedir="${4}"

    if ! [[ -d $(dirname "${ebuildfile}" ) ]] ; then
        die "dirname(ebuildfile) \"$(dirname "${ebuildfile}" )\" is not a directory"
    fi

    if [[ -e "${ebuildfile}" ]] && ! [[ -f "${ebuildfile}" ]] ; then
        die "ebuildfile \"${ebuildfile}\" exists and is not a regular file"
    fi

    ebuild_section "Loading saved environment..."

    ebuild_safe_source "${envfile}"

    ebuild_section "Writing binary ebuild to '${ebuildfile}'..."

    make_binary_ebuild \
        "${ebuildfile}" \
        "${PALUDIS_BINARY_URI_PREFIX}""$(basename ${bindistfile} ).tar.bz2" \
        "${PALUDIS_BINARY_KEYWORDS}"

    ebuild_section "Finished writing binary"

    true
}

main $@

