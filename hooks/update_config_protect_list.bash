#!/bin/bash
# vim: set et sw=4 sts=4 :

vdb_loc=$(${PALUDIS_COMMAND} --configuration-variable installed location )
cfg_protect_list="${vdb_loc}/.cache/all_CONFIG_PROTECT"
cfg_protect_mask_list="${vdb_loc}/.cache/all_CONFIG_PROTECT_MASK"

if [[ ! -f "${cfg_protect_list}" || ! -f "${cfg_protect_mask_list}" ]] ; then
    # Generate this list for the first time. *slow*
    echo "Creating the CONFIG_PROTECT and CONFIG_PROTECT_MASK lists."
    echo "This will take a while."
    > "${cfg_protect_list}"
    > "${cfg_protect_mask_list}"

    installed_pkgs=$(${PALUDIS_COMMAND} --list-packages --repository installed |grep ^* |cut -d" " -f2)

    for p in ${installed_pkgs} ; do
        cfg_protect=$(${PALUDIS_COMMAND} --environment-variable ${p} CONFIG_PROTECT)
        for x in ${cfg_protect} ; do
            echo "${x}" >> "${cfg_protect_list}"
        done

        cfg_protect_mask=$(${PALUDIS_COMMAND} --environment-variable ${p} CONFIG_PROTECT_MASK)
        for x in ${cfg_protect_mask} ; do
            echo "${x}" >> "${cfg_protect_mask_list}"
        done
    done
fi

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
