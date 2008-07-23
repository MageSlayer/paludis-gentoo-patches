#!/usr/bin/env bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2007 Ciaran McCreesh
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

unalias -a
set +C
unset GZIP BZIP BZIP2 CDPATH GREP_OPTIONS GREP_COLOR GLOBIGNORE
eval unset LANG ${!LC_*}

shopt -s expand_aliases
shopt -s extglob

if ! source $1 ; then
    echo "Error sourcing '$1' for hook '$HOOK'" 1>&2
    exit 123
fi

if [[ $(type -t $2 2>/dev/null ) != "function" ]] ; then
    if [[ ${2#hook_depend} != ${2} ]] ; then
        exit 0
    elif [[ ${2#hook_after} != ${2} ]] ; then
        exit 0
    else
        echo "Error running undefined function '$2' from file '$1' for hook '$HOOK'" 1>&2
        exit 124
    fi
fi

$2

