#!/bin/bash
# vim: set et sw=4 sts=4 :

# Copyright (c) 2007 Ciaran McCreesh
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

export PATH="$(${PALUDIS_EBUILD_DIR}/utils/canonicalise ${PALUDIS_EBUILD_DIR}/utils/ ):${PATH}"
source ${PALUDIS_EBUILD_DIR}/echo_functions.bash

[[ -n "${PALUDIS_NO_WRITE_CACHE_CLEAN}" ]] && exit 0

echo
einfo_unhooked "Cleaning write cache for ebuild format repositories..."

while read repo ; do
    wcloc=$(${PALUDIS_COMMAND} --configuration-variable ${repo} write_cache )

    [[ $(canonicalise ${wcloc} ) == "/var/empty" ]] && continue
    wcloc="${wcloc}/${repo}"

    [[ $(echo "${wcloc}"/* ) != "${wcloc}/*" ]] || continue
    echo rm -fr "${wcloc}/*"
    rm -fr "${wcloc}"/* || eerror "Couldn't clear cache for ${repo} at ${wcloc}"

done < <(${PALUDIS_COMMAND} --list-repositories --repository-format ebuild | \
    sed -n -e '/^\*/s,^\*\s*,,p' )

einfo_unhooked "Done cleaning write cache for ebuild format repositories"

