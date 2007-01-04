#!/bin/bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

EXPORT_FUNCTIONS()
{
    [[ -z "${ECLASS}" ]] && die "EXPORT_FUNCTIONS called but ECLASS undefined"

    local e
    for e in "$@" ; do
        case "$e" in
            pkg_setup|pkg_prerm|pkg_postrm|pkg_preinst|pkg_postinst)
                eval "${e}() { ${ECLASS}_${e} \"\$@\" ; }"
                ;;

            src_unpack|src_compile|src_install|src_test)
                eval "${e}() { ${ECLASS}_${e} \"\$@\" ; }"
                ;;

            *)
                eval "${e}() { ${ECLASS}_${e} \"\$@\" ; }"
                ebuild_notice "qa" "$e should not be in EXPORT_FUNCTIONS for ${ECLASS}"
                ;;
        esac
    done
}

inherit()
{
    [[ -n "${PALUDIS_SKIP_INHERIT}" ]] && return

    local e ee location=
    for e in "$@" ; do
        for ee in ${ECLASSDIRS:-${ECLASSDIR}} ; do
            [[ -f "${ee}/${e}.eclass" ]] && location="${ee}/${e}.eclass"
        done
        local old_ECLASS="${ECLASS}"
        export ECLASS="${e}"

        local current_IUSE="${IUSE}" current_DEPEND="${DEPEND}"
        local current_RDEPEND="${RDEPEND}" current_PDEPEND="${PDEPEND}"
        local current_KEYWORDS="${KEYWORDS}"

        unset IUSE DEPEND RDEPEND PDEPEND KEYWORDS

        [[ -z "${location}" ]] && die "Error finding eclass ${e}"
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

