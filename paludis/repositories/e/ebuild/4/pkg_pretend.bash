#!/usr/bin/env bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2008 David Leverton
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

default_pkg_pretend()
{
    :
}

pkg_pretend()
{
    default_pkg_pretend
}

ebuild_f_pretend()
{
    if hasq "pretend" ${SKIP_FUNCTIONS} ; then
        ebuild_section "Skipping pkg_pretend (SKIP_FUNCTIONS)"
    else
        echo
        pkg_pretend
        echo
    fi
    true
}

