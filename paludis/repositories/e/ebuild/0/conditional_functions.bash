#!/usr/bin/env bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2006, 2007, 2008 Ciaran McCreesh
# Copyright (c) 2011 David Leverton
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

use_with()
{
    if useq "${1}" ; then
        if [[ -n ${PALUDIS_USE_WITH_ENABLE_EMPTY_THIRD_ARGUMENT} ]]; then
            echo "--with-${2:-${1}}${3+=${3}}"
        else
            echo "--with-${2:-${1}}${3:+=${3}}"
        fi
    else
        echo "--without-${2:-${1}}"
    fi
}

use_enable()
{
    if useq "${1}" ; then
        if [[ -n ${PALUDIS_USE_WITH_ENABLE_EMPTY_THIRD_ARGUMENT} ]]; then
            echo "--enable-${2:-${1}}${3+=${3}}"
        else
            echo "--enable-${2:-${1}}${3:+=${3}}"
        fi
    else
        echo "--disable-${2:-${1}}"
    fi
}

