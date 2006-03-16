#!/bin/bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

builtin_tidyup()
{
    if [[ -e "${PALUDIS_TMPDIR}/${CATEGORY}/${PF}" ]] ; then
        rm -fr "${PALUDIS_TMPDIR}/${CATEGORY}/${PF}" || die "Couldn't remove work"
    fi
}

ebuild_f_tidyup()
{
    ebuild_section "Starting builtin_tidyup"
    builtin_tidyup
    ebuild_section "Done builtin_tidyup"
}


