#!/bin/bash
# vim: set et sw=4 sts=4 :

# Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

shopt -s extglob

export PATH="$(${PALUDIS_EBUILD_DIR}/utils/canonicalise ${PALUDIS_EBUILD_DIR}/utils/ ):${PATH}"
source ${PALUDIS_EBUILD_DIR}/echo_functions.bash

count=0
echo
einfo "Checking for broken symlinks..."

find "${IMAGE}" -type l | while read link ; do
    target=$(readlink "${link}" )
    if [[ "${target#${IMAGE}}" != "${target}" ]] ; then
        ebuild_notice "qa" "Found broken symlink '${link}' -> '${target}'"
        echo rm -f "${link}" 1>&2
        rm -f "${link}"
        newtarget="${target#${IMAGE}}"
        newtarget="/${newtarget##+(/)}"
        echo ln -s "${newtarget}" "${link}" 1>&2
        ln -s "${newtarget}" "${link}"
        count=$(( ${count} + 1 ))
    fi
done

if [[ ${count} == 0 ]] ; then
    einfo "Done checking for broken symlinks"
else
    ewarn "Fixed ${count} broken symlinks"
fi

true

