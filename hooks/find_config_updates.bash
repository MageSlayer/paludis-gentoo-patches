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

source /sbin/functions.sh

echo
einfo "Searching for configuration files requiring action..."

dir_count=0
for dir in /etc ${CONFIG_PROTECT} ; do
    [[ -d "${ROOT%/}/${dir}" ]] || continue
    if [[ -n $(find "${ROOT%/}/${dir}" -iname '._cfg????_*' ) ]] ; then
        einfo "Found files in ${ROOT%/}/${dir}"
        dir_count=$((dir_count + 1))
    fi
done

if [[ 0 -eq "${dir_count}" ]] ; then
    einfo "No configuration file updates required"
    exit 0
else
    ewarn "Found files in ${dir_count} directories"
    ewarn "Your action is required"
    exit 0
fi

