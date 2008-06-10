#!/bin/bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2006, 2007, 2008 Ciaran McCreesh
# Copyright (c) 2008 Bo Ørsted Andresen
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

nonfatal()
{
    PALUDIS_FAILURE_IS_FATAL= PALUDIS_FAILURE_IS_NONFATAL=yes "${@}"
}

expatch()
{
    if [[ "${EBUILD_PHASE}" != "prepare" ]] ; then
        die "expatch called in EBUILD_PHASE ${EBUILD_PHASE}"
    fi

    local recognise= patchlevel= options=() cmd= appliedpatches=0

    if [[ ${1} == "--recognised-suffixes" ]]; then
        recognise=true
        shift
    fi

    while [[ $# -gt 0 ]]; do
        if [[ ${1} == -p*([[:digit:]]) ]]; then
            patchlevel="${1}"
        elif [[ ${1} == -+([^[:space:]]) ]]; then
            options+=("${1}")
        elif [[ -d ${1} ]]; then
            expatch --recognised-suffixes ${patchlevel} "${options[@]}" "${1}"/* || return 247
            ((appliedpatches++))
        else
            case "${1}" in
                *.bz2)
                cmd="bzip2 -dc"
                ;;
                *.gz|*.Z|*.z)
                cmd="gzip -dc"
                ;;
                *.zip|*.ZIP|*.jar)
                cmd="unzip -p"
                ;;
                *.diff|*.patch)
                cmd="cat"
                ;;
                *)
                if [[ -n ${recognise} ]]; then
                    continue
                else
                    cmd="cat"
                fi
                ;;
            esac

            echo "${cmd} -- '${1}' | patch -s -f ${patchlevel:--p1} ${options[@]}" 1>&2
            ${cmd} -- "${1}" | patch -s -f ${patchlevel:--p1} "${options[@]}"
            paludis_assert_unless_nonfatal "applying '${1}' failed" || return 247
            ((appliedpatches++))
        fi
        shift
    done
    # Die if no patches were applied and no directories were supplied. Since
    # directories get handled recursively by separate instances of expatch we cannot
    # reliably count applied patches when directories were supplied.
    [[ ${appliedpatches} -gt 0 || -n ${recognise} ]] || paludis_die_unless_nonfatal "No patches applied." || return 247
}

econf()
{
    if [[ "${EBUILD_PHASE}" != "configure" ]] ; then
        die "econf called in EBUILD_PHASE ${EBUILD_PHASE}"
    fi

    local LOCAL_EXTRA_ECONF="${EXTRA_ECONF}"
    local LOCAL_ECONF_WRAPPER="${ECONF_WRAPPER}"

    [[ -z "${ECONF_SOURCE}" ]] && ECONF_SOURCE=.

    if [[ -x "${ECONF_SOURCE}/configure" ]] ; then
        if [[ -d /usr/share/gnuconfig ]] ; then
            local f
            find "${WORKDIR}" -type f -name config.guess -or -name config.sub | while read f; do
                echo "econf: updating ${f} with /usr/share/gnuconfig/${f##*/}"
                cp -f "/usr/share/gnuconfig/${f##*/}" "${f}"
            done
        fi

        [[ -z "${CBUILD}" ]] || LOCAL_EXTRA_ECONF="--build=${CBUILD} ${LOCAL_EXTRA_ECONF}"
        [[ -z "${CTARGET}" ]] || LOCAL_EXTRA_ECONF="--target=${CTARGET} ${LOCAL_EXTRA_ECONF}"

        # If the ebuild passed in --prefix, use that to set --libdir. KDE at least needs this.

        ECONF_PREFIX=/usr
        for i in "$@"; do
            if [[ ${i} == --prefix=* ]]; then
                ECONF_PREFIX=${i#--prefix=}
            elif [[ ${i} == --exec-prefix=* ]]; then
                ECONF_PREFIX=${i#--exec-prefix=}
            fi
        done

        local libcmd=
        if [[ -n "${ABI}" ]] ; then
            local v="LIBDIR_${ABI}"
            if [[ -n "${!v}" ]] ; then
                libcmd="--libdir=${ECONF_PREFIX}/$(ebuild_get_libdir)"
            fi
        fi

        echo ${LOCAL_ECONF_WRAPPER} "${ECONF_SOURCE}"/configure \
            --prefix=/usr \
            --host=${CHOST} \
            --mandir=/usr/share/man \
            --infodir=/usr/share/info \
            --datadir=/usr/share \
            --sysconfdir=/etc \
            --localstatedir=/var/lib \
            ${libcmd} "$@" ${LOCAL_EXTRA_ECONF} 1>&2

        ${LOCAL_ECONF_WRAPPER} "${ECONF_SOURCE}"/configure \
            --prefix=/usr \
            --host=${CHOST} \
            --mandir=/usr/share/man \
            --infodir=/usr/share/info \
            --datadir=/usr/share \
            --sysconfdir=/etc \
            --localstatedir=/var/lib \
            ${libcmd} "$@" ${LOCAL_EXTRA_ECONF} || paludis_die_unless_nonfatal "econf failed" || return 247

    else
        paludis_die_unless_nonfatal "No configure script for econf" || return 247
    fi
}

einstall()
{
    if [[ "${EBUILD_PHASE}" != "install" ]] ; then
        die "einstall called in EBUILD_PHASE ${EBUILD_PHASE}"
    fi

    if [[ -f Makefile ]] || [[ -f makefile ]] || [[ -f GNUmakefile ]] ; then
        local makecmd=""
        type -p gmake &>/dev/null && makecmd="gmake" || makecmd="make"
        local cmd="${EINSTALL_WRAPPER} ${makecmd} prefix=${D}/usr"
        cmd="${cmd} mandir=${D}/usr/share/man"
        cmd="${cmd} infodir=${D}/usr/share/info"
        cmd="${cmd} datadir=${D}/usr/share"
        cmd="${cmd} sysconfdir=${D}/etc"
        cmd="${cmd} localstatedir=${D}/var/lib"
        cmd="${cmd} libdir=${D}/usr/$(ebuild_get_libdir)"
        cmd="${cmd} ${EXTRA_EINSTALL} ${@} install"
        echo "${cmd}" 1>&2
        ${cmd} || paludis_die_unless_nonfatal "einstall failed" || return 247
    else
        paludis_die_unless_nonfatal "No Makefile for einstall" || return 247
    fi
}

