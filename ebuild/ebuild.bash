#!/bin/bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2006 Ciaran McCreesh <ciaranm@gentoo.org>
#
# Based in part upon ebuild.sh from Portage, which is Copyright 1995-2005
# Gentoo Foundation and distributed under the terms of the GNU General
# Public License v2.
#
# This file is part of the Paludis package manager. Paludis is free software;
# you can redistribute it and/or modify it under the terms of the GNU General
# Public License as published by the Free Software Foundation; either version
# 2 of the License, or (at your option) any later version.
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
unset GZIP BZIP BZIP2 CDPATH GREP_OPTIONS GREP_COLOR GLOBIGNORE
eval unset LANG ${!LC_*}

shopt -s expand_aliases

EBUILD_KILL_PID=$$
alias die='diefunc "$FUNCNAME" "$LINENO"'
alias assert='_pipestatus="${PIPESTATUS[*]}"; [[ -z "${_pipestatus//[ 0]/}" ]] || diefunc "$FUNCNAME" "$LINENO" "$_pipestatus"'
trap 'echo "exiting with error." ; exit 250' 15

diefunc()
{
    local func="$1" line="$2"
    shift 2
    echo 1>&2
    echo "ERROR in ${CATEGORY:-?}/${PF:-?}:" 1>&2
    echo "!!! In ${func:-?} at line ${line:-?}" 1>&2
    echo "!!! ${*:-(no message provided)}" 1>&2
    echo 1>&2

    kill ${EBUILD_KILL_PID}
    exit 249
}

EBUILD_MODULES_DIR=$(readlink -f $(dirname $0 ) )
[[ -d ${EBUILD_MODULES_DIR} ]] || die "${EBUILD_MODULES_DIR} is not a directory"

ebuild_load_module()
{
    source "${EBUILD_MODULES_DIR}/${1}.bash" || die "Error loading module ${1}"
}

ebuild_load_module sandbox_stubs
ebuild_load_module portage_stubs
ebuild_load_module list_functions
ebuild_load_module echo_functions
ebuild_load_module multilib_functions
ebuild_load_module install_functions
ebuild_load_module build_functions
ebuild_load_module unpack_functions
ebuild_load_module eclass_functions

ebuild_load_ebuild()
{
    [[ -f "${1}" ]] || die "Ebuild '${1}' is not a file"
    source ${1} || die "Error sourcing ebuild '${1}'"
    [[ ${RDEPEND-unset} == "unset" ]] && RDEPEND="${DEPEND}"
}

case ${1:x} in
    metadata)
        ebuild_load_ebuild "${2}"
        ebuild_load_module depend
        ebuild_f_depend
        exit 0
        ;;

    unpack|compile|install|test)
        ebuild_load_ebuild "${2}"
        ebuild_load_module src_${1}
        ebuild_f_${1}
        exit 0
    ;;

    setup|config|nofetch|preinst|postinst|prerm|postrm)
        ebuild_load_ebuild "${2}"
        ebuild_load_module pkg_${1}
        ebuild_f_${1}
        exit 0
    ;;

    *)
        ebuild_load_module usage_error
        ebuild_f_usage_error
        exit 1
    ;;
esac

