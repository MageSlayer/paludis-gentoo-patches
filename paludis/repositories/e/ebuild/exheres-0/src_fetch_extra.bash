#!/usr/bin/env bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2009 Ciaran McCreesh
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

default_src_fetch_extra()
{
    :
}

src_fetch_extra()
{
    default "$@"
}

exheres_internal_fetch_extra()
{
    cd "${WORK}" || die "cd to \${WORK} (\"${WORK}\") failed"

    if hasq "fetch_extra" ${SKIP_FUNCTIONS} ; then
        ebuild_section "Skipping src_fetch_extra (SKIP_FUNCTIONS)"
    else
        ebuild_section "Starting src_fetch_extra"
        src_fetch_extra
        ebuild_section "Done src_fetch_extra"
    fi
}


