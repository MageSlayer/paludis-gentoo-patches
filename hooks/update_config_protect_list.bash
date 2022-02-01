#!/usr/bin/env bash
# vim: set et sw=4 sts=4 :

# Copyright (c) 2006 Mike Kelly
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

source "${PALUDIS_ECHO_FUNCTIONS_DIR:-${PALUDIS_EBUILD_DIR}}/echo_functions.bash"

if [[ -n "${PALUDIS_NO_LIVE_DESTINATION}" ]] ; then
    einfo_unhooked "No need to update CONFIG_PROTECT lists"
    exit 0
fi

vdb_loc=$(${CAVE:-cave} print-repository-metadata installed --raw-name location --format '%v' )
cfg_protect_list="${vdb_loc}/.cache/all_CONFIG_PROTECT"
cfg_protect_mask_list="${vdb_loc}/.cache/all_CONFIG_PROTECT_MASK"

if [[ ! -f "${cfg_protect_list}" || ! -f "${cfg_protect_mask_list}" ]] ; then
    # Generate this list for the first time. *slow*
    ewarn "Creating the CONFIG_PROTECT and CONFIG_PROTECT_MASK lists."
    ewarn "This will take a while, but we only have to do it once."
    [[ -d "${vdb_loc}/.cache" ]] || mkdir ${vdb_loc}/.cache || return 1
    > "${cfg_protect_list}"
    > "${cfg_protect_mask_list}"

    cfg_protect=$(${CAVE:-cave} print-id-metadata --all '*/*'::installed --raw-name CONFIG_PROTECT --format '%v' | xargs -n1 | uniq )
    for x in ${cfg_protect} ; do
        echo "${x}" >> "${cfg_protect_list}"
    done

    cfg_protect_mask=$(${CAVE:-cave} print-id-metadata --all '*/*'::installed --raw-name CONFIG_PROTECT_MASK --format '%v' | xargs -n1 | uniq )
    for x in ${cfg_protect_mask} ; do
        echo "${x}" >> "${cfg_protect_mask_list}"
    done
fi

einfo "Updating CONFIG_PROTECT and CONFIG_PROTECT_MASK caches."

# Now, update the lists with our current values.
for x in ${CONFIG_PROTECT} ; do
    echo "${x}" >> "${cfg_protect_list}"
done

for x in ${CONFIG_PROTECT_MASK} ; do
    echo "${x}" >> "${cfg_protect_mask_list}"
done

# Finally, ensure we have no duplicates in these lists.
sort -u "${cfg_protect_list}" > "${cfg_protect_list}.$$"
mv "${cfg_protect_list}.$$" "${cfg_protect_list}"

sort -u "${cfg_protect_mask_list}" > "${cfg_protect_mask_list}.$$"
mv "${cfg_protect_mask_list}.$$" "${cfg_protect_mask_list}"
