#!/bin/bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

econf()
{
    local LOCAL_EXTRA_ECONF="${EXTRA_ECONF}"
    local LOCAL_ECONF_WRAPPER="${ECONF_WRAPPER}"

    [[ -z "${ECONF_SOURCE}" ]] && ECONF_SOURCE=.

    if [[ -x "${ECONF_SOURCE}/configure" ]] ; then
        if [[ -d /usr/share/gnuconfig ]] ; then
            local f
            for f in $(find "${WORKDIR}" -type f -name config.guess -or -name config.sub ) ; do
                echo "econf: updating ${f} with /usr/share/gnuconfig/${f##*/}"
                cp -f /usr/share/gnuconfig/${f##*/} ${f}
            done
        fi

        [[ -z "${CBUILD}" ]] || LOCAL_EXTRA_ECONF="--build=${CBUILD} ${LOCAL_EXTRA_ECONF}"
        [[ -z "${CTARGET}" ]] || LOCAL_EXTRA_ECONF="--target=${CTARGET} ${LOCAL_EXTRA_ECONF}"

        local cmd="${ECONF_SOURCE}/configure"
        cmd="${cmd} --prefix=/usr"
        cmd="${cmd} --host=${CHOST}"
        cmd="${cmd} --mandir=/usr/share/man"
        cmd="${cmd} --infodir=/usr/share/info"
        cmd="${cmd} --datadir=/usr/share"
        cmd="${cmd} --sysconfdir=/etc"
        cmd="${cmd} --localstatedir=/var/lib"
        # Check that this is actually what's wanted for multilib etc.
        if [[ -n "${ABI}" ]] ; then
            local v="LIBDIR_${ABI}"
            if [[ -n "${!v}" ]] ; then
                cmd="${cmd} --libdir=/usr/$(ebuild_get_libdir)"
            fi
        fi

        cmd="${LOCAL_ECONF_WRAPPER} ${cmd} $@ ${LOCAL_EXTRA_ECONF}"

        echo "${cmd}" 1>&2
        ${cmd} || die "econf failed"

    else
        die "No configure script for econf"
    fi
}

emake()
{
    echo ${EMAKE_WRAPPER} ${MAKE:-make} ${MAKEOPTS} ${EXTRA_EMAKE} "$@" 1>&2
    ${EMAKE_WRAPPER} ${MAKE:-make} ${MAKEOPTS} ${EXTRA_EMAKE} "$@"
}

einstall()
{
    if [[ -f Makefile ]] || [[ -f makefile ]] || [[ -f GNUmakefile ]] ; then
        local cmd="${EINSTALL_WRAPPER} make prefix='${D}/usr'"
        cmd="${cmd} mandir='${D}/usr/share/man'"
        cmd="${cmd} infodir='${D}/usr/share/info'"
        cmd="${cmd} datadir='${D}/usr/share'"
        cmd="${cmd} sysconfdir='${D}/etc'"
        cmd="${cmd} localstatedir='${D}/var/lib'"
        cmd="${cmd} ${EXTRA_EINSTALL} ${@} install"
        echo "${cmd}" 1>&2
        ${cmd} || die "einstall failed"
    else
        die "No Makefile for einstall"
    fi
}

