#!/usr/bin/env bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2006, 2007, 2008, 2009 Ciaran McCreesh
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
                exit 123
            fi
            return
        else
            t="${t:+${t}, }${d}"
        fi
    done

    [[ -n "${older}" ]] && export "${excl_v}"="${save_excl}"

    type die &>/dev/null && eval die "\"Couldn't find module \${1} (looked in \${t})\""
    echo "Couldn't find module ${1} (looked in ${t})" 1>&2
    exit 123
}

ebuild_load_module die_functions
ebuild_load_module output_functions
ebuild_load_module echo_functions
ebuild_load_module source_functions

export PALUDIS_HOME="$(canonicalise ${PALUDIS_HOME:-${HOME}} )"

main()
{
    local vdbdir="${1}" envfile="${2}"

    if ! [[ -d "${vdbdir}" ]] ; then
        echo "!!! vdbdir \"${vdbdir}\" is not a directory"
        exit 1
    fi

    ebuild_section "Writing VDB entry to '${vdbdir}'..."

    ebuild_safe_source "${envfile}"

    ebuild_section "Writing VDB entry keys ..."

    if [[ -z "${PALUDIS_VDB_FROM_ENV_VARIABLES}" ]] ; then
        ebuild_notice "warning" "VDB_FROM_ENV_VARIABLES not set, using defaults"
        PALUDIS_VDB_FROM_ENV_VARIABLES="\
            CATEGORY CHOST COUNTER DEPEND DESCRIPTION EAPI \
            FEATURES HOMEPAGE INHERITED IUSE KEYWORDS LICENSE PDEPEND PF \
            PROVIDE RDEPEND SLOT SRC_URI USE CONFIG_PROTECT CONFIG_PROTECT_MASK \
            VDB_FORMAT PKGMANAGER"
    fi

    if [[ -z "${PALUDIS_VDB_FROM_ENV_UNLESS_EMPTY_VARIABLES}" ]] ; then
        ebuild_notice "warning" "PALUDIS_VDB_FROM_ENV_UNLESS_EMPTY_VARIABLES not set, using defaults"
        PALUDIS_VDB_FROM_ENV_UNLESS_EMPTY_VARIABLES="\
            ASFLAGS CBUILD CC CFLAGS CHOST CTARGET CXX CXXFLAGS \
            EXTRA_ECONF EXTRA_EINSTALL EXTRA_EMAKE LDFLAGS LIBCXXFLAGS \
            REPOSITORY"
    fi

    local v VDB_FORMAT="paludis-2" COUNTER="$(date +%s )"
    for v in ${PALUDIS_VDB_FROM_ENV_VARIABLES} ; do
        if ! paludis_rewrite_var VDB "${v}" "${!v}" > "${vdbdir}"/${v} ; then
            echo "!!! vdb write ${v} failed"
            exit 1
        fi
    done

    for v in ${PALUDIS_VDB_FROM_ENV_UNLESS_EMPTY_VARIABLES} ; do
        [[ -z "${!v}" ]] && continue
        if ! paludis_rewrite_var VDB "${v}" "${!v}" > "${vdbdir}"/${v} ; then
            echo "!!! vdb write ${v} failed"
            exit 1
        fi
    done

    ebuild_section "Generating saved ebuild and environment..."

    [[ -f "${EBUILD}" ]] && cp "${EBUILD}" ${vdbdir}/
    bzip2 < ${envfile} > ${vdbdir}/environment.bz2

    ebuild_section "Finished writing VDB entry"

    true
}

main $@

