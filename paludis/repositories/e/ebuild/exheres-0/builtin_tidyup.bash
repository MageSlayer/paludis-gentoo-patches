#!/usr/bin/env bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2006, 2008 Ciaran McCreesh
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

builtin_tidyup()
{
    if [[ -z ${PALUDIS_PACKAGE_BUILDDIR} ]]; then
        local a
        for a in PALUDIS_TMPDIR CATEGORY PNVR  ; do
            [[ -z "${!a}" ]] && die "\$${a} unset or empty"
        done
        PALUDIS_PACKAGE_BUILDDIR=${PALUDIS_TMPDIR}/${CATEGORY}-${PNVR}
    fi

    if [[ -e "${PALUDIS_PACKAGE_BUILDDIR}" ]] ; then
        if type -p chflags &>/dev/null; then
            echo chflags -R 0 "${PALUDIS_PACKAGE_BUILDDIR}" 1>&2
            chflags -R 0 "${PALUDIS_PACKAGE_BUILDDIR}" || die "Couldn't remove flags from workdir"
        fi
        [[ -z ${PALUDIS_EBUILD_QUIET} ]] && echo rm -fr "${PALUDIS_PACKAGE_BUILDDIR}" 1>&2
        rm -fr "${PALUDIS_PACKAGE_BUILDDIR}" || die "Couldn't remove work"
    fi
}

exheres_internal_tidyup()
{
    if hasq "tidyup" ${SKIP_FUNCTIONS} ; then
        ebuild_section "Skipping builtin_tidyup (SKIP_FUNCTIONS)"
    else
        ebuild_section "Starting builtin_tidyup"
        builtin_tidyup
        ebuild_section "Done builtin_tidyup"
    fi
}

ebuild_f_tidyup()
{
    exheres_internal_tidyup ""
}

