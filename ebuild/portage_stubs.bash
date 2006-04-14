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

has_version()
{
    ${PALUDIS_COMMAND} --has-version "$@"
}

portageq()
{
    # \todo Make this suck less...
    if [[ "$1" == "has_version" ]] ; then
        if [[ "$(readlink -f $2 )" != "$(readlink -f $ROOT )" ]] ; then
            eerror "Error emulating 'portageq $@':"
            die "portageq has_version emulation only works on current ROOT"
        else
            shift ; shift
            has_version "$@"
        fi
    elif [[ "$1" == "best_version" ]] ; then
        if [[ "$(readlink -f $2 )" != "$(readlink -f $ROOT )" ]] ; then
            eerror "Error emulating 'portageq $@':"
            die "portageq best_version emulation only works on current ROOT"
        else
            shift ; shift
            best_version "$@"
        fi
    else
        eerror "Error emulating 'portageq $@':"
        die "portageq emulation for $1 not implemented"
    fi
}

best_version()
{
    ${PALUDIS_COMMAND} --best-version "$@"
}

check_KV()
{
    die "check_KV not implenented"
}

debug-print()
{
    :
}

debug-print-function()
{
    :
}

