#!/bin/bash
# vim: set et sw=4 sts=4 :

# Copyright (c) 2007 Fernando J. Pereda <ferdy@gentoo.org>
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

einfo_unhooked "Removing setuid and setgid bits..."

while read file ; do
    [[ -f "${file}" ]] || continue
    if [[ -u "${file}" ]] || [[ -g "${file}" ]] ; then
        chmod gu-s "${file}"
    fi
done < <(${PALUDIS_COMMAND} --contents =${CATEGORY}/${PF} |
            sed -e '1d' -e '/^\s*$/d' -e 's-^\s*--')

einfo_unhooked "Done removing setuid and setgid bits."

true
