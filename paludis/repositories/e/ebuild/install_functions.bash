#!/usr/bin/env bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2006 Ciaran McCreesh
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

umask 022
export DESTTREE="/usr"
export INSDESTTREE=""
export EXEDESTTREE=""
export DOCDESTTREE=""
export INSOPTIONS="-m0644"
export EXEOPTIONS="-m0755"
export LIBOPTIONS="-m0644"
export DIROPTIONS="-m0755"
export MOPREFIX="${PN}"

keepdir()
{
    local keepfile_name=.keep_${CATEGORY}_${PN}-${SLOT%/*}
    if [[ ${#} -lt 1 ]]; then
        paludis_die_unless_nonfatal "at least one argument needed"
    fi
    if [[ ${@} != ${@#${!PALUDIS_IMAGE_DIR_VAR}} ]]; then
        paludis_die_unless_nonfatal "You should not use \${${PALUDIS_IMAGE_DIR_VAR}} with helpers."
    fi
    dodir "$@"
    if [[ "${1}" == "-R" ]] || [[ "${1}" == "-r" ]] ; then
        shift
        find "$@" -type d -printf "${!PALUDIS_IMAGE_DIR_VAR}/%p/${keepfile_name}\0" | xargs -0 touch
        paludis_assert_unless_nonfatal "Failed to create ${keepfile_name} files" || return 247
    else
        local f
        for f in "$@" ; do
            touch "${!PALUDIS_IMAGE_DIR_VAR}/${f}/${keepfile_name}" || paludis_die_unless_nonfatal "Couldn't touch ${keepfile_name} in ${f}" || return 247
        done
    fi
}

into()
{
    if [[ "${1}" == "/" ]] ; then
        export DESTTREE=
    else
        export DESTTREE="${1}"
        [[ -d "${!PALUDIS_IMAGE_DIR_VAR}${DESTTREE}" ]] || install -d "${!PALUDIS_IMAGE_DIR_VAR}${DESTTREE}"
    fi
}

insinto()
{
    if [[ "${1}" == "/" ]] ; then
        export INSDESTTREE=
    else
        export INSDESTTREE="${1}"
        [[ -d "${!PALUDIS_IMAGE_DIR_VAR}${INSDESTTREE}" ]] || install -d "${!PALUDIS_IMAGE_DIR_VAR}${INSDESTTREE}"
    fi
}

exeinto()
{
    if [[ "${1}" == "/" ]] ; then
        export EXEDESTTREE=
    else
        export EXEDESTTREE="${1}"
        [[ -d "${!PALUDIS_IMAGE_DIR_VAR}${EXEDESTTREE}" ]] || install -d "${!PALUDIS_IMAGE_DIR_VAR}${EXEDESTTREE}"
    fi
}

docinto()
{
    if [[ "${1}" == "/" ]] ; then
        export DOCDESTTREE=
    else
        export DOCDESTTREE="${1}"
        [[ -d "${!PALUDIS_IMAGE_DIR_VAR}usr/share/doc/${!PALUDIS_NAME_VERSION_REVISION_VAR}/${DOCDESTTREE}" ]] || \
            install -d "${!PALUDIS_IMAGE_DIR_VAR}usr/share/doc/${!PALUDIS_NAME_VERSION_REVISION_VAR}/${DOCDESTTREE}"
    fi
}

insopts()
{
    export INSOPTIONS="$@"
}

diropts()
{
    export DIROPTIONS="$@"
}

exeopts()
{
    export EXEOPTIONS="$@"
}

libopts()
{
    export LIBOPTIONS="$@"
}

