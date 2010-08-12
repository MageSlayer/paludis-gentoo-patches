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

portageq()
{
    # \todo Make this suck less...
    if [[ "$1" == "has_version" ]] ; then
        if [[ "$(canonicalise $2 )" != "$(canonicalise $ROOT )" ]] ; then
            eerror "Error emulating 'portageq $@':"
            die "portageq has_version emulation only works on current ROOT"
        else
            shift ; shift
            has_version "$@"
        fi
    elif [[ "$1" == "best_version" ]] ; then
        if [[ "$(canonicalise $2 )" != "$(canonicalise $ROOT )" ]] ; then
            eerror "Error emulating 'portageq $@':"
            die "portageq best_version emulation only works on current ROOT"
        else
            shift ; shift
            best_version "$@"
        fi
    elif [[ "$1" == "vdb_path" ]] ; then
        vdb_path
    elif [[ "$1" == "match" ]] ; then
        if [[ "$(canonicalise $2 )" != "$(canonicalise $ROOT )" ]] ; then
            eerror "Error emulating 'portageq $@':"
            die "portageq match emulation only works on current ROOT"
        else
            shift ; shift
            [[ "${#@}" -ne 1 ]] && die "match should take exactly one arg"
            local r=$(paludis_pipe_command MATCH "$EAPI" "$1" ) m=""
            echo "${r#*;}"
            return ${r%%;*}
        fi
    else
        eerror "Error emulating 'portageq $@':"
        die "portageq emulation for $1 not implemented"
    fi
}

best_version()
{
    [[ "${#@}" -ne 1 ]] && die "$0 should take exactly one arg"
    local r=$(paludis_pipe_command BEST_VERSION "$EAPI" --root "$1" )
    echo ${r#*;}
    return ${r%%;*}
}

has_version()
{
    [[ "${#@}" -ne 1 ]] && die "$0 should take exactly one arg"
    local r=$(paludis_pipe_command HAS_VERSION "$EAPI" --root "$1" )
    return ${r%%;*}
}

vdb_path()
{
    [[ "${#@}" -ne 0 ]] && die "vdb_path should take exactly zero args"
    local r=$(paludis_pipe_command VDB_PATH "$EAPI" "$1" )
    echo ${r#*;}
    return ${r%%;*}
}

check_KV()
{
    die "check_KV not implemented"
}

debug-print()
{
    :
}

debug-print-function()
{
    :
}

debug-print-section()
{
    :
}

