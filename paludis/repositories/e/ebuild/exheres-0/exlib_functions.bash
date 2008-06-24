#!/bin/bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2007, 2008 Ciaran McCreesh
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
        case "$e" in
            pkg_nofetch|pkg_setup|pkg_prerm|pkg_postrm|pkg_preinst|pkg_postinst|pkg_config|pkg_pretend)
                eval "${e}() { ${CURRENT_EXLIB}_${e} \"\$@\" ; }"
                ;;

            src_unpack|src_prepare|src_configure|src_compile|src_install|src_test)
                eval "${e}() { ${CURRENT_EXLIB}_${e} \"\$@\" ; }"
                ;;

            *)
                die "$e should not be in export_exlib_phases for ${CURRENT_EXLIB}"
                ;;
        esac
    done
}

require()
{
    ebuild_notice "debug" "Command 'require ${@}', using EXLIBSDIRS '${EXLIBSDIRS}'"
    local e ee location= v
    for e in "$@" ; do
        for ee in ${EXLIBSDIRS} ; do
            [[ -f "${ee}/${e}.exlib" ]] && location="${ee}/${e}.exlib"
        done
        local old_CURRENT_EXLIB="${CURRENT_EXLIB}"
        export CURRENT_EXLIB="${e}"

        for v in ${PALUDIS_SOURCE_MERGED_VARIABLES} ${PALUDIS_BRACKET_MERGED_VARIABLES} ; do
            local c_v="current_${v}" u_v="unset_${v}"
            local ${c_v}="${!v}"
            local ${u_v}="${!v-unset}"
            unset ${v}
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
            export -n ${e_v}="${!e_v} ( ${!v} )"
        done

        for v in ${PALUDIS_SOURCE_MERGED_VARIABLES} ${PALUDIS_BRACKET_MERGED_VARIABLES} ; do
            local c_v="current_${v}" u_v="unset_${v}"
            [[ "unset" == ${!u_v} ]] && unset ${v} || export ${v}="${!c_v}"
        done

        export CURRENT_EXLIB="${old_CURRENT_EXLIB}"
    done
}

default()
{
    [[ $(type -t "default_$(paludis_phase_to_function_name "${EBUILD_PHASE}")" ) == "function" ]] || \
        die "default_$(paludis_phase_to_function_name "${EBUILD_PHASE}") is not a function"
    default_$(paludis_phase_to_function_name "${EBUILD_PHASE}") "$@"
}
