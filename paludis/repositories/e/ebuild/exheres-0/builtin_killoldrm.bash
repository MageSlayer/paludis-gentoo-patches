#!/usr/bin/env bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2006, 2007, 2008 Ciaran McCreesh
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

builtin_killoldrm()
{
    local a
    for a in CATEGORY PALUDIS_TMPDIR  ; do
        [[ -z "${!a}" ]] && die "\$${a} unset or empty"
    done
    [[ -z "${PF}" && -z "${PNVR}" ]] && die "PF and PNVR both unset or empty"

    if [[ -e "${PALUDIS_TMPDIR}/${CATEGORY}-${PNVR:-${PF}}-uninstall" ]] ; then
        if type -p chflags &>/dev/null; then
            chflags -R 0 "${PALUDIS_TMPDIR}/${CATEGORY}-${PNVR:-${PF}}-uninstall" || die "Couldn't remove flags from workdir"
        fi
        rm -fr "${PALUDIS_TMPDIR}/${CATEGORY}-${PNVR:-${PF}}-uninstall" || die "Couldn't remove previous work"
    fi
}

exheres_internal_killoldrm()
{
    if hasq "killoldrm" ${SKIP_FUNCTIONS} ; then
        ebuild_section "Skipping builtin_killoldrm (SKIP_FUNCTIONS)"
    else
        ebuild_section "Starting builtin_killoldrm"
        builtin_killoldrm
        ebuild_section "Done builtin_killoldrm"
    fi
}

ebuild_f_killoldrm()
{
    exheres_internal_killoldrm ""
}

