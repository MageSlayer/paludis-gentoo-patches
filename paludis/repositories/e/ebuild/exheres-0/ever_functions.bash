#!/usr/bin/env bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2008, 2009 Ciaran McCreesh
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
            [[ "${#@}" != 2 ]] && die "$FUNCNAME $1 takes one extra argument"
            r=$(paludis_pipe_command EVER "$EAPI" SPLIT "${2:-${PV}}" )
        ;;

        split_all)
            [[ "${#@}" != 2 ]] && die "$FUNCNAME $1 takes one extra argument"
            r=$(paludis_pipe_command EVER "$EAPI" SPLIT_ALL "${2:-${PV}}" )
        ;;

        major)
            [[ "${#@}" != 1 ]] && [[ "${#@}" != 2 ]] && die "$FUNCNAME $1 takes zero or one extra arguments"
            r=$(paludis_pipe_command EVER "$EAPI" RANGE 1 "${2:-${PV}}" )
        ;;

        range)
            [[ "${#@}" != 2 ]] && [[ "${#@}" != 3 ]] && die "$FUNCNAME $1 takes one or two extra arguments"
            r=$(paludis_pipe_command EVER "$EAPI" RANGE ${2} "${3:-${PV}}" )
        ;;

        remainder)
            [[ "${#@}" != 1 ]] && [[ "${#@}" != 2 ]] && die "$FUNCNAME $1 takes at most one extra argument"
            r=$(paludis_pipe_command EVER "$EAPI" RANGE 2- "${2:-${PV}}" )
        ;;

        replace)
            [[ "${#@}" != 3 ]] && [[ "${#@}" != 4 ]] && die "$FUNCNAME $1 takes two or three extra arguments"
            [[ -z "${2}" ]] && die "$FUNCNAME $1 takes a non-empty position as first argument"
            r=$(paludis_pipe_command EVER "$EAPI" REPLACE "${2}" "${3}" "${4:-${PV}}" )
        ;;

        replace_all)
            [[ "${#@}" != 2 ]] && [[ "${#@}" != 3 ]] && die "$FUNCNAME $1 takes one or two extra arguments"
            r=$(paludis_pipe_command EVER "$EAPI" REPLACE_ALL "${2}" "${3:-${PV}}" )
        ;;

        delete)
            [[ "${#@}" != 2 ]] && [[ "${#@}" != 3 ]] && die "$FUNCNAME $1 takes one or two extra arguments"
            [[ -z "${2}" ]] && die "$FUNCNAME $1 takes a non-empty position as first argument"
            r=$(paludis_pipe_command EVER "$EAPI" REPLACE "${2}" "" "${3:-${PV}}" )
        ;;

        delete_all)
            [[ "${#@}" != 1 ]] && [[ "${#@}" != 2 ]] && die "$FUNCNAME $1 takes at most one extra argument"
            r=$(paludis_pipe_command EVER "$EAPI" REPLACE_ALL "" "${2:-${PV}}" )
        ;;

        at_least)
            [[ "${#@}" != 2 ]] && [[ "${#@}" != 3 ]] && die "$FUNCNAME $1 takes one or two extra arguments"
            r=$(paludis_pipe_command EVER "$EAPI" AT_LEAST "${2}" "${3:-${PVR}}" )
        ;;

        is_scm)
            [[ "${#@}" != 1 ]] && [[ "${#@}" != 2 ]] && die "$FUNCNAME $1 takes zero or one extra arguments"
            r=$(paludis_pipe_command EVER "$EAPI" IS_SCM "${2:-${PV}}" )
        ;;

        *)
            die "ever subcommand ${1} unrecognised"
        ;;
    esac

    [[ -z "${r#*;}" ]] || echo "${r#*;}"
    return ${r%%;*}
}

