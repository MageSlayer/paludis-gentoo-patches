#!/bin/bash
# vim: set et sw=4 sts=4 :

# Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

shopt -s extglob

export PATH="$(${PALUDIS_EBUILD_DIR}/utils/canonicalise ${PALUDIS_EBUILD_DIR}/utils/ ):${PATH}"
source ${PALUDIS_EBUILD_DIR}/echo_functions.bash

if [[ -n "${PALUDIS_NO_LIVE_DESTINATION}" ]] ; then
    einfo_unhooked "No need to search for configuration files requiring action"
    exit 0
fi

echo
einfo_unhooked "Searching for configuration files requiring action..."

dir_count=0
for dir in /etc ${CONFIG_PROTECT} ; do
    [[ -d "${ROOT%/}/${dir}" ]] || continue
    c=$(find "${ROOT%/}/${dir}" -iname '._cfg????_*' | wc -l )
    if [[ ${c} -gt 0 ]] ; then
        einfo_unhooked "Found ${c} files in ${ROOT%%+(/)}/${dir}"
        dir_count=$((dir_count + 1))
    fi
done

if [[ 0 -eq "${dir_count}" ]] ; then
    einfo_unhooked "No configuration file updates required"
    exit 0
else
    ewarn "Found files in ${dir_count} directories"
    ewarn "Your action is required"
    exit 0
fi

