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

use_with()
{
    if useq "${1}" ; then
        echo "--with-${2:-${1}}${3+=${3}}"
    else
        echo "--without-${2:-${1}}"
    fi
}

use_enable()
{
    if useq "${1}" ; then
        echo "--enable-${2:-${1}}${3+=${3}}"
    else
        echo "--disable-${2:-${1}}"
    fi
}

ebuild_notice_level()
{
    case "$1" in
        debug)
            echo "1";
            ;;

        qa)
            echo "2";
            ;;

        warning)
            echo "3";
            ;;

        silent)
            echo "4";
            ;;

        *)
            echo "[WARNING.EBUILD] Bad value '$1' for qa level" 1>&2
            echo "2";
            ;;
    esac
}

ebuild_notice()
{
    local level="$1"
    shift

    local level_num=$(ebuild_notice_level "${level}" )
    local min_level_num=$(ebuild_notice_level "${PALUDIS_EBUILD_LOG_LEVEL}" )

    if [[ "${level_num}" -ge "${min_level_num}" ]] ; then
        local upper_level=$(echo ${level} | ${ebuild_real_tr} '[:lower:]' '[:upper:]' )
        echo "[${upper_level}.EBUILD] $* (from ${EBUILD})" 1>&2
    fi
    true
}

ebuild_section()
{
    echo ">>> $*"
}

