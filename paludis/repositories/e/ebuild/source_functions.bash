#!/bin/bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2007 David Leverton
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

ebuild_safe_source()
{
    set -- "${@}" \
        EUID PPID UID FUNCNAME GROUPS SHELLOPTS \
        'BASH_@(ARGC|ARGV|LINENO|SOURCE|VERSINFO|REMATCH)' \
        'BASH_COMPLETEION?(_DIR)' 'bash+([0-9])?([a-z])' \
        EBUILD_KILL_PID PALUDIS_LOADSAVEENV_DIR PALUDIS_DO_NOTHING_SANDBOXY SANDBOX_ACTIVE

    trap DEBUG
    set -T
    shopt -s extdebug
    trap "[[ \${BASH_COMMAND%% *} == @(eval|trap) ||
              ( \${BASH_COMMAND} != *([^\$'\n'])=* && \${BASH_COMMAND} != export\ * ) ||
              \${BASH_COMMAND} != ?(export\ )@($(IFS='|'; shift; echo "${*}"))?(=*) ]]" DEBUG

    source "${1}"
    eval "trap DEBUG; shopt -u extdebug; set +T; return ${?}"
}

