#!/usr/bin/env bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2008 Ciaran McCreesh
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

builtin_installbin()
{
    echo tar jvxpf ${!PALUDIS_BINARY_DISTDIR_VARIABLE}/${A} -C "${IMAGE}"/ --exclude PBIN 1>&2
    tar jvxpf ${!PALUDIS_BINARY_DISTDIR_VARIABLE}/${A} -C "${IMAGE}"/ --exclude PBIN || die "Couldn't extract image"
}

generic_internal_installbin()
{
    if hasq "init" ${SKIP_FUNCTIONS} ; then
        ebuild_section "Skipping builtin_installbin (SKIP_FUNCTIONS)"
    else
        ebuild_section "Starting builtin_installbin"
        builtin_installbin
        ebuild_section "Done builtin_installbin"
    fi
}

ebuild_f_installbin()
{
    generic_internal_installbin "$@"
}

exheres_internal_installbin()
{
    generic_internal_installbin "$@"
}

