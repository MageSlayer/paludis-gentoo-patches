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

unpack()
{
    [[ -z "$@" ]] && die "No arguments given to unpack"

    for x in "$@" ; do
        echo ">>> Unpacking ${x} to ${PWD}"
        [[ "${x:0:2}" == "./" ]] || x="${DISTDIR}/${x}"
        unpack_one "${x}"
    done
}

unpack_one()
{
    [[ -z "$1" ]] && die "Bad argument for unpack_one"
    [[ -e "$1" ]] || die "${1} doesn't exist"

    case "${x}" in
        *.tar)
            tar xf "${1}" || die "Couldn't unpack ${1}"
            ;;

        *.tar.gz|*.tgz)
            tar zxf "${1}" || die "Couldn't unpack ${1}"
            ;;

        *.tar.bz2|*.tbz2
            tar jxf "${1}" || die "Couldn't unpack ${1}"
            ;;

        *.zip|*.ZIP|*.jar)
            unzip -qo "${1}" || die "Couldn't unpack ${1}"
            ;;

        *.gz|*.Z|*.z)
            gzip -dc "${1}" > "${1%.*}" || die "Couldn't unpack ${1}"
            ;;

        *.bz2)
            bunzip2 -dc "${1}" > "${1%.*}" || die "Couldn't unpack ${1}"
            ;;

        *.rar|*.RAR)
            unrar x -idq "${1}" || die "Couldn't unpack ${1}"
            ;;

        *.LHa|*.LHA|*.lha|*.lzh)
            lha xqf "${1}" || die "Couldn't unpack ${1}"
            ;;

        *)
            echo "Skipping unpack for ${1}"
            ;;
    esac

}

