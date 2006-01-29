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

unalias -a
unset GZIP BZIP BZIP2 CDPATH GREP_OPTIONS GREP_COLOR GLOBIGNORE
eval unset LANG ${!LC_*}

shopt -s expand_aliases

EBUILD_KILL_PID=$$
alias die='diefunc "$FUNCNAME" "$LINENO" "$@"'
alias assert='_pipestatus="${PIPESTATUS[*]}"; [[ -z "${_pipestatus//[ 0]/}" ]] || diefunc "$FUNCNAME" "$LINENO" "$_pipestatus"'

diefunc()
{
    local func="$1" line="$2" exitcode="$3"
    shift 3
    echo 1>&2
    echo "ERROR in ${CATEGORY}/${PF}:" 1>&2
    echo "!!! $funcname at $lineno gave exit code $exitcode" 1>&2
    echo "!!! ${@:-(no message provided)}" 1>&2
    echo 1>&2
}

umask 022
export DESTTREE="/usr"
export INSDESTTREE=""
export EXEDESTTREE=""
export DOCDESTTREE=""
export INSOPTIONS="-m0644"
export EXEOPTIONS="-m0755"
export LIBOPTIONS="-m0644"
export DIROPTIONS="-m0755"
export MOPREFIX="${PN}"

source sandbox_stubs.bash
source portage_stubs.bash
source list_functions.bash
source echo_functions.bash
source install_functions.bash
