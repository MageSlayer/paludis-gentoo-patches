#!/bin/bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2008 Ciaran McCreesh
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

ever()
{
    local r="1;"
    case "${1}" in
        split)
        ;;

        split_all)
        ;;

        major)
        ;;

        range)
        ;;

        remainder)
        ;;

        replace_separator)
        ;;

        replace_all_separators)
        ;;

        delete_separator)
        ;;

        delete_all_separators)
        ;;

        at_least)
            [[ "${#@}" != 2 ]] && [[ "${#@}" != 3 ]] && die "$0 $1 takes one or two extra arguments"
            r=$(paludis_pipe_command EVER "$EAPI" AT_LEAST "${2}" "${3:-${PVR}}" )
        ;;

        *)
        die "ever subcommand ${1} unrecognised"
        ;;
    esac

    [[ -z "${r#*;}" ]] || echo "${r#*;}"
    return ${r%%;*}
}
