#!/usr/bin/env bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2006, 2007 Ciaran McCreesh
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

has_version()
{
    illegal_in_global_scope
    [[ "${#@}" -ne 1 ]] && die "$0 should take exactly one arg"
    local r=$(paludis_pipe_command HAS_VERSION "$EAPI" "$1" )
    return ${r%%;*}
}

portageq()
{
    die "Function 'portageq' banned in this EAPI"
}

best_version()
{
    illegal_in_global_scope
    [[ "${#@}" -ne 1 ]] && die "$0 should take exactly one arg"
    local r=$(paludis_pipe_command BEST_VERSION "$EAPI" "$1" )
    echo ${r#*;}
    return ${r%%;*}
}

vdb_path()
{
    die "Function 'vdb_path' banned in this EAPI"
}

check_KV()
{
    die "check_KV not implemented"
}

debug-print()
{
    die "Function 'debug-print' banned in this EAPI"
}

debug-print-function()
{
    die "Function 'debug-print-function' banned in this EAPI"
}

debug-print-section()
{
    die "Function 'debug-print-section' banned in this EAPI"
}
