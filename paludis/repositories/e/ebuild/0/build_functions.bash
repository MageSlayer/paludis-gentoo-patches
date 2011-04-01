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

econf()
{
    local phase
    has src_configure ${PALUDIS_EBUILD_FUNCTIONS} && phase=configure
    if [[ "${!PALUDIS_EBUILD_PHASE_VAR}" != "${phase:-compile}" ]]; then
        ebuild_notice "qa" "econf called in EBUILD_PHASE ${!PALUDIS_EBUILD_PHASE_VAR}. It should not be run outside src_${phase} for this EAPI."
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

        for i in ${PALUDIS_ECONF_EXTRA_OPTIONS_HELP_DEPENDENT}; do
            "${ECONF_SOURCE}/configure" --help 2>/dev/null | grep -q "${i%%::*}" \
                && LOCAL_EXTRA_ECONF+=" ${i#*::}"
        done

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
            ${PALUDIS_ECONF_EXTRA_OPTIONS} \
            ${libcmd} "$@" ${LOCAL_EXTRA_ECONF} 1>&2

        ${LOCAL_ECONF_WRAPPER} "${ECONF_SOURCE}"/configure \
            --prefix=/usr \
            --host=${CHOST} \
            --mandir=/usr/share/man \
            --infodir=/usr/share/info \
            --datadir=/usr/share \
            --sysconfdir=/etc \
            --localstatedir=/var/lib \
            ${PALUDIS_ECONF_EXTRA_OPTIONS} \
            ${libcmd} "$@" ${LOCAL_EXTRA_ECONF} || die "econf failed"

    else
        die "No configure script for econf"
    fi
}

einstall()
{
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
        ${cmd} || die "einstall failed"
    else
        die "No Makefile for einstall"
    fi
}

