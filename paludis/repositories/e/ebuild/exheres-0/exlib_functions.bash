#!/usr/bin/env bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2007, 2008, 2009 Ciaran McCreesh
# Copyright (c) 2009 Bo Ørsted Andresen
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

export_exlib_phases()
{
    [[ -z "${CURRENT_EXLIB}" ]] && die "export_exlib_phases called but CURRENT_EXLIB undefined"

    local e
    for e in "$@" ; do
        if [[ "${e}" == builtin_* ]] || ! has "${e}" ${PALUDIS_EBUILD_FUNCTIONS}; then
            die "$e should not be in export_exlib_phases for ${CURRENT_EXLIB}"
        fi
        eval "${e}() { ${CURRENT_EXLIB}_${e} \"\$@\" ; }"
    done
}

exparam()
{
    die "exparam is banned outside exlibs"
}

exparam_var_name()
{
    echo EXPARAMVAR_${1//-/__dash__}
}

exparam_print()
{
    case "${1}" in
        -a) eval "${2}=( \"\${${3}}\" )" ;;
        -v) eval "${2}=\"\${${3}}\""      ;;
        *)  eval "echo \"\${${1}}\""     ;;
    esac
}

exparam_internal()
{
    local i e=${1} a to_var v a_v=$(exparam_var_name ${1})__ALLDECLS__ b_v=$(exparam_var_name ${1})__BOOLEANS__
    shift

    if [[ ${1} == -b ]]; then
        [[ ${#} -eq 2 ]] || die "exparam ${1} requires exactly two arguments"
        shift
        has "${1}" ${!b_v} || die "${e}.exlib has no ${1} boolean"
        v=$(exparam_var_name ${e})_${1}
        if ${!v}; then
            return 0
        else
            return 1
        fi
    fi

    if [[ ${1} == -v ]]; then
        [[ ${#} -eq 3 ]] || die "exparam ${1} requires exactly three arguments"
        a=${1}
        to_var=${2}
        shift 2
    else
        [[ ${#} -eq 1 ]] || die "exparam requires exactly one argument"
    fi

    v=$(exparam_var_name ${e})_${1%\[*}
    if [[ ${1} == *\[*\] ]]; then
        has "${1%\[*}[]" ${!a_v} || die "${e}.exlib has no ${1%\[*} array"
        i=${1#*\[}
        i=${i%\]}
        case "${i}" in
            "#")            exparam_print ${to_var:+-v "${to_var}"} "#${v}[*]"   ;;
            "*"|"@")        exparam_print ${to_var:+-a "${to_var}"} "${v}[${i}]"    ;;
            +([[:digit:]])) exparam_print ${to_var:+-v "${to_var}"} "${v}[${i}]" ;;
            *)              die "Invalid index in exparam ${1}"          ;;
        esac
    else
        has "${1}" ${!a_v%\[\]} || die "${e}.exlib has no ${1} parameter"
        exparam_print ${to_var:+-v "${to_var}"} "${v}"
    fi
}

myexparam()
{
    [[ -z "${CURRENT_EXLIB}" ]] && die "myexparam called but CURRENT_EXLIB undefined"

    local bool=false
    if [[ ${1} == -b ]]; then
        bool=true
        shift
    fi

    local v=${1%%=*} a_v="$(exparam_var_name ${CURRENT_EXLIB})__ALLDECLS__"
    [[ ${1} == *=\[ && ${#} -gt 1 ]] && v+="[]"
    printf -v "${a_v}" "%s %s" "${!a_v}" "${v}"
    if ${bool}; then
        local b_v="$(exparam_var_name ${CURRENT_EXLIB})__BOOLEANS__"
        printf -v "${b_v}" "%s %s" "${!b_v}" "${v}"
    fi

    v=$(exparam_var_name ${CURRENT_EXLIB})_${v%\[\]}
    if [[ -z ${!v+set} && ${1} == *=* ]]; then
        if [[ ${1} == *=\[ && ${#} -gt 1 ]]; then
            shift
            local i a=()
            while [[ ${#} -gt 1 ]]; do
                a+=( "${1}" )
                shift
            done
            [[ ${1} == \] ]] || die "Array encountered with no closing ]"
            eval "${v}=( \"\${a[@]}\" )"
        else
            printf -v "${v}" "%s" "${1#*=}"
        fi
    fi

    ${bool} && ! has "${!v}" true false && die "exparam ${1%%=*} for exlib ${CURRENT_EXLIB} must be 'true' or 'false'"
}

require()
{
    ebuild_notice "debug" "Command 'require ${@}', using EXLIBSDIRS '${EXLIBSDIRS}'"
    local exlibs e ee p a=() location v a_v v_qa
    # parse exlib parameters
    while [[ -n $@ ]]; do
        if [[ ${1} == +('[') ]]; then
            [[ -z ${e} ]] && die "\"${1}\" encountered with no preceding exlib"
            p=${1}
            shift
            while [[ -n ${1} && ${1} != ${p//\[/\]} ]]; do
                v="${1%%=*}"
                if [[ ${1#*=} == ${p} ]]; then
                    v+="[]"
                    a=()
                    shift
                    while [[ -n ${1} && ${1} != ${p//\[/\]} ]]; do
                        a+=( "${1}" )
                        shift
                    done
                    [[ ${1} == ${p//\[/\]} ]] || die "\"${p}\" encountered with no closing \"${p//[/]}\" for array ${v}"
                    eval "$(exparam_var_name ${e})_${v%\[\]}=( \"\${a[@]}\" )"
                else
                    printf -v "$(exparam_var_name ${e})_${1%%=*}" "%s" "${1#*=}"
                fi
                a_v="$(exparam_var_name ${e})__ALL__"
                printf -v "${a_v}" "%s %s" "${!a_v}" "${v}"
                shift
            done
            [[ ${1} == ${p//\[/\]} ]] || die "\"${p}\" encountered with no closing \"${p//[/]}\""
        else
            e=${1}
            exlibs+=" ${e}"
        fi
        shift
    done
    # source exlibs
    for e in ${exlibs}; do
        location=
        for ee in ${EXLIBSDIRS} ; do
            [[ -f "${ee}/${e}.exlib" ]] && location="${ee}/${e}.exlib"
        done
        local old_CURRENT_EXLIB="${CURRENT_EXLIB}"
        export CURRENT_EXLIB="${e}"
        alias exparam="exparam_internal ${CURRENT_EXLIB}"

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

        [[ -z "${location}" ]] && die "Error finding exlib ${e} in ${EXLIBSDIRS}"
        source "${location}" || die "Error sourcing exlib ${e}"
        hasq "${CURRENT_EXLIB}" ${INHERITED} || export INHERITED="${INHERITED} ${CURRENT_EXLIB}"

        for v in ${PALUDIS_SOURCE_MERGED_VARIABLES} ; do
            local e_v="E_${v}"
            export -n ${e_v}="${!e_v} ${!v}"
        done

        for v in ${PALUDIS_BRACKET_MERGED_VARIABLES} ; do
            local e_v="E_${v}"
            if [[ -z "${!v}" ]] ; then
                export -n ${e_v}="${!e_v}"
            elif has "${v}" ${PALUDIS_BRACKET_MERGED_VARIABLES_ANNOTATABLE} ; then
                export -n ${e_v}="${!e_v} ( ${!v} ) [[ ${PALUDIS_BRACKET_MERGED_VARIABLES_ANNOTATION} =
                    [ ${CURRENT_EXLIB}.exlib ] ]]"
            else
                export -n ${e_v}="${!e_v} ( ${!v} )"
            fi
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

        # die on required exlib parameters that haven't been supplied
        local c_v v
        a_v=$(exparam_var_name ${CURRENT_EXLIB})__ALLDECLS__
        for v in ${!a_v}; do
            c_v=$(exparam_var_name ${CURRENT_EXLIB})_${v%\[\]}
            if [[ $(eval "declare -p ${c_v}") == declare\ -a\ ${c_v}=* ]]; then
                [[ ${v} == *\[\] ]] || die "${CURRENT_EXLIB}.exlib requires a scalar ${v} parameter but got an array"
            elif [[ -n ${!c_v+set} ]]; then
                [[ ${v} != *\[\] ]] || die "${CURRENT_EXLIB}.exlib requires an array ${v} but got a scalar"
            else
                die "${CURRENT_EXLIB}.exlib requires a ${v} parameter"
            fi
        done

        # die on supplied exlib parameters which haven't been declared
        v=$(exparam_var_name ${CURRENT_EXLIB})__ALL__
        for v in ${!v}; do
            has ${v} ${!a_v} || die "${CURRENT_EXLIB}.exlib takes no ${v} parameter"
        done

        export CURRENT_EXLIB="${old_CURRENT_EXLIB}"
        if [[ -n ${CURRENT_EXLIB} ]]; then
            alias exparam="exparam_internal ${CURRENT_EXLIB}"
        else
            unalias exparam
        fi
    done
}

default()
{
    [[ $(type -t "default_$(paludis_phase_to_function_name "${!PALUDIS_EBUILD_PHASE_VAR}")" ) == "function" ]] || \
        die "default_$(paludis_phase_to_function_name "${!PALUDIS_EBUILD_PHASE_VAR}") is not a function"
    default_$(paludis_phase_to_function_name "${!PALUDIS_EBUILD_PHASE_VAR}") "$@"
}

illegal_in_global_scope()
{
    [[ "${!PALUDIS_EBUILD_PHASE_VAR}" == "metadata" ]] \
        && die "Exheres bug: ${FUNCNAME[1]} must not be called in global scope"
}

verify_not_called_cross_phase() {
    if [[ ${1:-${FUNCNAME[1]}} != $(paludis_phase_to_function_name "${!PALUDIS_EBUILD_PHASE_VAR}") ]] ; then
        local correct_phase=${1:-${FUNCNAME[1]}#src_}; correct_phase=${correct_phase#pkg_}; correct_phase=${correct_phase#builtin_}
        die "Exheres bug: ${FUNCNAME[1]} must only be called in the ${corect_phase} phase, was called in ${!PALUDIS_EBUILD_PHASE_VAR}"
    fi
}

