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

EXPORT_FUNCTIONS()
{
    [[ -z "${ECLASS}" ]] && die "EXPORT_FUNCTIONS called but ECLASS undefined"

    local e
    for e in "$@" ; do
        eval "$1() { ${ECLASS}_${e} \"\$@\" ; }"
    done
}

inherit()
{
    local e
    for e in "$@" ; do
        local location="${ECLASSDIR}/${e}.eclass"
        local old_ECLASS="${ECLASS}"
        export ECLASS="${e}"

        local current_IUSE="${IUSE}" current_DEPEND="${DEPEND}"
        local current_RDEPEND="${RDEPEND}" current_PDEPEND="${PDEPEND}"
        local current_KEYWORDS="${KEYWORDS}"

        unset IUSE DEPEND RDEPEND PDEPEND KEYWORDS

        source "${location}" || die "Error sourcing eclass ${e}"
        hasq "${ECLASS}" ${INHERITED} || export INHERITED="${INHERITED} ${ECLASS}"

        E_IUSE="${E_IUSE} ${IUSE}"
        E_PDEPEND="${E_PDEPEND} ${PDEPEND}"
        E_RDEPEND="${E_RDEPEND} ${RDEPEND}"
        E_DEPEND="${E_DEPEND} ${DEPEND}"
        E_KEYWORDS="${KEYWORDS:+${KEYWORDS} }${E_KEYWORDS}"

        IUSE="${current_IUSE}"
        DEPEND="${current_DEPEND}"
        RDEPEND="${current_RDEPEND}"
        PDEPEND="${current_PDEPEND}"
        KEYWORDS="${current_KEYWORDS}"

        export ECLASS="${old_ECLASS}"
    done
}

