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

typeset -ix _paludis_eclass_level='0'
EXPORT_FUNCTIONS()
{
    [[ -z "${ECLASS}" ]] && die "EXPORT_FUNCTIONS called but ECLASS undefined"

    local e
    for e in "$@" ; do
        if [[ ${!PALUDIS_EBUILD_PHASE_VAR} != metadata ]] && { [[ "${e}" == builtin_* ]] || ! has "${e}" ${PALUDIS_EBUILD_FUNCTIONS}; }; then
            ebuild_notice "qa" "$e should not be in EXPORT_FUNCTIONS for ${ECLASS}"
        fi
        eval '_paludis_func_export_'"${_paludis_eclass_level}"'+=("${e}")'
    done
}

inherit()
{
    [[ -n "${PALUDIS_SKIP_INHERIT}" ]] && return

    _paludis_eclass_level="$((++_paludis_eclass_level))"

    # Reset/initialize functions export variable for the current level.
    eval 'typeset -a _paludis_func_export_'"${_paludis_eclass_level}"'=()'

    local e ee location v v_qa
    for e in "$@" ; do
        location=
        for ee in ${ECLASSDIRS:-${ECLASSDIR}} ; do
            [[ -f "${ee}/${e}.eclass" ]] && location="${ee}/${e}.eclass"
        done
        local old_ECLASS="${ECLASS}"
        export ECLASS="${e}"

        for v in ${PALUDIS_SOURCE_MERGED_VARIABLES} ${PALUDIS_BRACKET_MERGED_VARIABLES} ; do
            local c_v="current_${v}" u_v="unset_${v}"
            local ${c_v}="${!v}"
            local ${u_v}="${!v-unset}"
            unset ${v}
        done

        for v_qa in ${PALUDIS_ECLASS_MUST_NOT_SET_VARIABLES} ; do
            local v=${v_qa#qa:}
            local c_v="current_${v}" u_v="unset_${v}"
            export -n ${c_v}="${!v}"
            export -n ${u_v}="${!v-unset}"
        done

        [[ -z "${location}" ]] && die "Error finding eclass ${e}"
        source "${location}" || die "Error sourcing eclass ${e}"
        hasq "${ECLASS}" ${INHERITED} || export INHERITED="${INHERITED} ${ECLASS}"

        for v in ${PALUDIS_SOURCE_MERGED_VARIABLES} ; do
            local e_v="E_${v}"
            export -n ${e_v}="${!e_v} ${!v}"
        done

        for v in ${PALUDIS_BRACKET_MERGED_VARIABLES} ; do
            local e_v="E_${v}"
            export -n ${e_v}="${!e_v} ( ${!v} )"
        done

        for v in ${PALUDIS_SOURCE_MERGED_VARIABLES} ${PALUDIS_BRACKET_MERGED_VARIABLES} ; do
            local c_v="current_${v}" u_v="unset_${v}"
            [[ "unset" == ${!u_v} ]] && unset ${v} || export ${v}="${!c_v}"
        done

        for v_qa in ${PALUDIS_ECLASS_MUST_NOT_SET_VARIABLES} ; do
            local v=${v_qa#qa:}
            local c_v="current_${v}" u_v="unset_${v}"
            if [[ ${!c_v} != ${!v} || ${!u_v} != ${!v-unset} ]]; then
                if [[ ${v} == ${v_qa} ]] ; then
                    die "Variable '${v}' illegally set by ${location}"
                else
                    ebuild_notice "qa" "Variable '${v}' should not be set by ${location}"
                    export -n ${c_v}="${!v}"
                    export -n ${u_v}="${!v-unset}"
                fi
            fi
        done

        eval '
        if [[ '"'"'0'"'"' -ne "${#_paludis_func_export_'"${_paludis_eclass_level}"'[@]}" ]]; then
            for func in "${_paludis_func_export_'"${_paludis_eclass_level}"'[@]}"; do
                # Check if function we want to call is actually available, like portage does.
                declare -F "${ECLASS}_${func}" >/dev/null || die "${ECLASS}_${func} not defined, but override requested"
                eval "${func}() { ${ECLASS}_${func} \"\$@\"; }" > /dev/null
            done
        fi
        unset '"'"'_paludis_func_export_'"${_paludis_eclass_level}'"

        export ECLASS="${old_ECLASS}"
    done

    _paludis_eclass_level="$((--_paludis_eclass_level))"
}

