#!/usr/bin/env bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2006, 2007, 2008 Ciaran McCreesh
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
    die "Function 'use_with' banned in this EAPI"
}

use_enable()
{
    die "Function 'use_enable' banned in this EAPI"
}

option_with()
{
    option "${1}" "--with-${2:-$(optionfmt ${1} )}${3+=${3}}" "--without-${2:-$(optionfmt ${1} )}"
}

option_enable()
{
    option "${1}" "--enable-${2:-$(optionfmt ${1} )}${3+=${3}}" "--disable-${2:-$(optionfmt ${1} )}"
}

