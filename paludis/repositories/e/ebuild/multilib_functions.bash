#!/usr/bin/env bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2006 Stephen Bennett
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

# Default values. Those used on a non-multilib profile.
export MULTILIB_ABIS=${MULTILIB_ABIS:-"default"}
export DEFAULT_ABI=${DEFAULT_ABI:-"default"}
export CFLAGS_default
export LDFLAGS_default
export CHOST_default=${CHOST_default:-${CHOST}}
export CTARGET_default=${CTARGET_default:-${CTARGET:-${CHOST_default}}}
export LIBDIR_default=${CONF_LIBDIR:-"lib"}
export CDEFINE_default


ebuild_get_libdir() {
    local CONF_LIBDIR
    if [[ -n "${CONF_LIBDIR_OVERRIDE}" ]]; then
        echo ${CONF_LIBDIR_OVERRIDE}
    else
        ebuild_get_abi_LIBDIR
    fi
}

ebuild_get_abi_var() {
    local flag=$1
    local abi
    if [[ $# -gt 1 ]]; then
        abi=${2}
    elif [[ -n "${ABI}" ]]; then
        abi=${ABI}
    elif [[ -n "${DEFAULT_ABI}" ]]; then
        abi=${DEFAULT_ABI}
    else
        abi="default"
    fi

    local var="${flag}_${abi}"
    echo ${!var}
}

ebuild_get_abi_CFLAGS() { ebuild_get_abi_var CFLAGS "$@"; }
ebuild_get_abi_LDFLAGS() { ebuild_get_abi_var LDFLAGS "$@"; }
ebuild_get_abi_CHOST() { ebuild_get_abi_var CHOST "$@"; }
ebuild_get_abi_CTARGET() { ebuild_get_abi_var CTARGET "$@"; }
ebuild_get_abi_FAKE_TARGETS() { ebuild_get_abi_var FAKE_TARGETS "$@"; }
ebuild_get_abi_CDEFINE() { ebuild_get_abi_var CDEFINE "$@"; }
ebuild_get_abi_LIBDIR() { ebuild_get_abi_var LIBDIR "$@"; }
