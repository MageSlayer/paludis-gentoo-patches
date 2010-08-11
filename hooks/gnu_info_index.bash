#!/usr/bin/env bash
# vim: set et sw=4 sts=4 :

# Copyright (c) 2006, 2007, 2008, 2009, 2010 Ciaran McCreesh
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
source ${PALUDIS_ECHO_FUNCTIONS_DIR:-${PALUDIS_EBUILD_DIR}}/echo_functions.bash

if [[ -n "${PALUDIS_NO_LIVE_DESTINATION}" ]] ; then
    einfo_unhooked "No need to update the GNU info directory"
    exit 0
fi

echo
einfo_unhooked "Checking whether the GNU info directory needs updating..."

# ticket:767
export INFOPATH=$(source /etc/profile.env 2>/dev/null ; echo $INFOPATH )

regen_info_dirs=
vdb_loc=$(${CAVE:-cave} print-repository-metadata installed --raw-name location --format '%v' )
for info_path in ${INFOPATH//:/ } ; do
    info_path="${ROOT%/}/${info_path}"
    [[ -d "${info_path}" ]] || continue
    info_time=$(wrapped_getmtime "${info_path}" )

    if [[ -f "${vdb_loc}/.cache/info_time_cache" ]] ; then
        info_time_cache=$(wrapped_getmtime "${vdb_loc}"/.cache/info_time_cache )
        [[ "${info_time}" -le "${info_time_cache}" ]] && continue
    fi

    regen_info_dirs="${regen_info_dirs} ${info_path}"
done

if [[ -z "${regen_info_dirs}" ]] ; then
    einfo_unhooked "No updates needed"
    exit 0
fi

good_count=0
bad_count=0

for info_path in ${regen_info_dirs} ; do
    einfo_unhooked "Updating directory ${info_path}..."

    [[ -e "${info_path}"/dir ]] && mv -f "${info_path}/"dir{,.old}

    for d in ${info_path}/* ; do
        [[ -f "${d}" ]] || continue
        [[ "${d}" != "${d%dir}" ]] && continue
        [[ "${d}" != "${d%dir.old}" ]] && continue

        is_bad=
        /usr/bin/install-info --quiet --dir-file="${info_path}/dir" "${d}" 2>&1 | \
        while read line ; do
            [[ "${line/already exists, for file/}" != "${line}" ]] && continue
            [[ "${line/warning: no info dir entry in /}" != "${line}" ]] && continue
            is_bad=probably
        done

        if [[ -n "${is_bad}" ]] ; then
            bad_count=$(( bad_count + 1 ))
        else
            good_count=$(( good_count + 1 ))
        fi
    done
done

touch "${vdb_loc}/.cache/info_time_cache"

if [[ ${bad_count} -gt 0 ]] ; then
    ewarn "Processed $(( good_count + bad_count )) info files, with ${bad_count} errors"
else
    einfo_unhooked "Processed ${good_count} info files"
fi

exit 0
