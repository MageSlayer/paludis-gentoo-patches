#!/bin/bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

builtin_updateenv()
{
    cd "${PALUDIS_TMPDIR}"
    echo eselect env update
    eselect env update || die "eselect env update failed"
}

ebuild_f_updateenv()
{
    if hasq "updateenv" ${RESTRICT} ; then
        ebuild_section "Skipping builtin_updateenv (RESTRICT)"
    elif hasq "tidyup" ${SKIP_FUNCTIONS} ; then
        ebuild_section "Skipping builtin_updateenv (SKIP_FUNCTIONS)"
    else
        ebuild_section "Starting builtin_updateenv"
        builtin_updateenv
        ebuild_section "Done builtin_updateenv"
    fi
}

