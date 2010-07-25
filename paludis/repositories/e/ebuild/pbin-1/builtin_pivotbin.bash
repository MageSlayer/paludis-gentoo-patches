#!/usr/bin/env bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2008, 2010 Ciaran McCreesh
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

builtin_pivotbin()
{
    [[ ! -d "${!PALUDIS_TEMP_DIR_VAR}" ]] && die "Can't use \${${PALUDIS_TEMP_DIR_VAR}}=${!PALUDIS_TEMP_DIR_VAR}"

    ebuild_section "Extracting package environment"
    echo tar jxvf "${!PALUDIS_BINARY_DISTDIR_VARIABLE}"/${!PALUDIS_ARCHIVES_VAR} -C "${!PALUDIS_TEMP_DIR_VAR}" --strip-components 1 PBIN/environment.bz2 1>&2
    tar jxvf "${!PALUDIS_BINARY_DISTDIR_VARIABLE}"/${!PALUDIS_ARCHIVES_VAR} -C "${!PALUDIS_TEMP_DIR_VAR}" --strip-components 1 PBIN/environment.bz2 || die "Couldn't extract env"

    ebuild_section "Switching to package environment"
    export BINARY_REPOSITORY="${REPOSITORY}"
    export PALUDIS_LOAD_ENVIRONMENT="${!PALUDIS_TEMP_DIR_VAR}/environment.bz2"
    ebuild_load_environment --pivot
    export EAPI="${EAPI#pbin-1+}"

    ebuild_section "Continuing using package environment"
}

generic_internal_pivotbin()
{
    if hasq "init" ${SKIP_FUNCTIONS} ; then
        ebuild_section "Skipping builtin_pivotbin (SKIP_FUNCTIONS)"
    else
        ebuild_section "Starting builtin_pivotbin"
        builtin_pivotbin
        ebuild_section "Done builtin_pivotbin"
    fi
}

ebuild_f_pivotbin()
{
    generic_internal_pivotbin "$@"
}

exheres_internal_pivotbin()
{
    generic_internal_pivotbin "$@"
}

