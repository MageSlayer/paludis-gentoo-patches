#!/bin/bash
# vim: set et sw=4 sts=4 :

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

export PATH="$(${PALUDIS_EBUILD_DIR}/utils/canonicalise ${PALUDIS_EBUILD_DIR}/utils/ ):${PATH}"
source ${PALUDIS_EBUILD_DIR}/echo_functions.bash

mkdir -p ${ROOT}/var/lib/paludis/news/

done_echo=

if [[ "${HOOK/pretend}" == "${HOOK}" ]] ; then
    [[ -z "${done_echo}" ]] && echo ; done_echo=yes
    einfo "Checking for news items..."

    ${PALUDIS_COMMAND} --update-news
fi

count=0
for f in "${ROOT}/var/lib/paludis/news/"news-*.unread ; do
    [[ -f "${f}" ]] || continue
    if grep --silent . "${f}" ; then
        count=$(( count + $(grep --count . < "${f}" ) ))
    fi
done

if [[ ${count} -gt 0 ]] ; then
    [[ -z "${done_echo}" ]] && echo ; done_echo=yes
    ewarn "You have ${count} unread news items"
    echo
elif [[ "${HOOK/pretend}" == "${HOOK}" ]] ; then
    [[ -z "${done_echo}" ]] && echo ; done_echo=yes
    einfo "No unread news items found"
fi

true

