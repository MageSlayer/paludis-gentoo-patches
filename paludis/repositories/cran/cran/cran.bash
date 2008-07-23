#!/usr/bin/env bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2006,2007 Danny van Dyk
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

. ${PALUDIS_EBUILD_DIR}/echo_functions.bash

shopt -s expand_aliases

CRAN_KILL_PID=$$
alias die='diefunc "$FUNCNAME" "$LINENO"'
trap 'echo "die trap: exiting with error." 1>&2 ; exit 250' 15

for f in ${PALUDIS_BASHRC_FILES} ; do
    if [[ -f ${f} ]] ; then
        ebuild_notice "debug" "Loading bashrc file ${f}"
        source ${f}
    else
        ebuild_notice "debug" "Skipping bashrc file ${f}"
    fi
done

diefunc()
{
    local func="$1" line="$2"
    shift 2
    echo 1>&2
    echo "!!! ERROR in cran/${PN:-?}:" 1>&2
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

    kill ${CRAN_KILL_PID}
    exit 249
}

cran_command() {
    local ret=1 command=${1}
    shift

    ebuild_section "Starting ${command}"

    if type cran_do_${command} &> /dev/null ; then
        cran_do_${command} || die "Failed to run command: ${command}"
    else
        die "Unknown command: ${command}"
    fi

    ebuild_section "Done ${command}"
}

cran_do_clean() {
    [[ -e ${WORKDIR} ]] && rm -rf ${WORKDIR}
    [[ -e ${IMAGE} ]] && rm -rf ${IMAGE}
    return 0
}

cran_do_fetch() {
    local mirror ret=0
    shift

    for mirror in ${PALUDIS_CRAN_MIRRORS[@]} ; do
        mkdir -p ${DISTDIR}
        cd ${DISTDIR}

        if [[ -e ${DISTFILE} ]] ; then
            echo "Already fetched: $(basename ${DISTFILE})"
            return 0
        fi

        local dofetch="${PALUDIS_EBUILD_DIR}/fetchers/do$(echo ${mirror%%://*})"
        local a="${mirror}/src/contrib/${DISTFILE}"
        if [[ -f "${dofetch}" ]] ; then
            ${dofetch} "${a}" "${DISTFILE}"
            ret=$?
        else
            eerror "Don't know how to fetch '${a}'"
            ret=1
        fi
        if [[ ${ret} == 0 ]] ; then
            break
        else
            echo "Failed to run \"${command}\" for mirror ${mirror}."
        fi
    done
    if [[ ${ret} != 0 ]] ; then
        echo "Finally failed to run \"${command}\"."
    fi
}

cran_do_install() {
    if [[ -e "${WORKDIR}" ]] ; then
        rm -Rf "${WORKDIR}" || die "Could not remove existing WORKDIR: ${WORKDIR}"
    fi
    mkdir -p "${WORKDIR}"
    cd "${WORKDIR}" || die "Could not cd to WORKDIR: ${WORKDIR}"

    local dounpack="${PALUDIS_EBUILD_DIR}/utils/dounpack"
    ${dounpack} "${DISTFILE}"

    # \todo Sandbox this?
    R CMD INSTALL -l "${IMAGE}/${PALUDIS_CRAN_LIBRARY##${ROOT}}" ${PN}

    if [[ ${IS_BUNDLE} == "yes" ]] ; then
        mkdir -p "${IMAGE}/${PALUDIS_CRAN_LIBRARY##${ROOT}}/paludis/bundles/"
        cp "${PN}/DESCRIPTION" "${IMAGE}/${PALUDIS_CRAN_LIBRARY##${ROOT}}/paludis/bundles/${PN}.DESCRIPTION"
    fi
}

cran_do_merge() {
    [[ -e "${IMAGE}/${INSTALL_TO##${ROOT}}/R.css" ]] \
        && rm -f "${IMAGE}/${PALUDIS_CRAN_LIBRARY##${ROOT}}/R.css"
    mkdir -p "${PALUDIS_CRAN_LIBRARY}/paludis/${PN}"
    ${PALUDIS_EBUILD_DIR}/merge "${IMAGE}" "${ROOT}" "${PALUDIS_CRAN_LIBRARY}/paludis/${PN}/CONTENTS"
    echo ${REPOSITORY} > "${PALUDIS_CRAN_LIBRARY}/paludis/${PN}/REPOSITORY"
}

cran_do_unmerge() {
    [[ -e "${PALUDIS_CRAN_LIBRARY}/paludis/${PN}/CONTENTS" ]] || die "CONTENTS file is missing for package ${PN}"
    ${PALUDIS_EBUILD_DIR}/unmerge "${ROOT}" "${PALUDIS_CRAN_LIBRARY}/paludis/${PN}/CONTENTS"
}

for cmd in $* ; do
    cran_command ${cmd}
done
