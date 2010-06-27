#!/usr/bin/env bash
# vim: set et sw=4 sts=4 :

# Copyright (c) 2006, 2008 Ciaran McCreesh
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

mkdir -p "${PALUDIS_LOG_DIRECTORY:-${ROOT}/var/log}"

if [[ -n ${X_OF_Y} ]] ; then
    COUNTS=" (${X_OF_Y})"
else
    COUNTS=" [${NUMBER_PENDING}p ${NUMBER_ACTIVE}a ${NUMBER_DONE}d]"
fi

(
    echo -n "$(date +%s ): "

    case "${HOOK}" in
        install_pre)
            echo "starting install of package ${TARGET}${COUNTS}"
        ;;

        install_post)
            echo "finished install of package ${TARGET}${COUNTS}"
        ;;

        install_all_pre)
            echo "starting install of targets ${TARGETS}"
        ;;

        install_all_post)
            echo "finished install of targets ${TARGETS}"
        ;;

        uninstall_pre)
            echo "starting uninstall of package ${TARGET}${COUNTS}"
        ;;

        uninstall_post)
            echo "finished uninstall of package ${TARGET}${COUNTS}"
        ;;

        uninstall_all_pre)
            echo "starting uninstall of targets ${TARGETS}"
        ;;

        uninstall_all_post)
            echo "finished uninstall of targets ${TARGETS}"
        ;;

        clean_pre)
            echo "starting clean of package ${TARGET}${COUNTS}"
        ;;

        clean_post)
            echo "finished clean of package ${TARGET}${COUNTS}"
        ;;

        clean_all_pre)
            echo "starting clean of targets ${TARGETS}"
        ;;

        clean_all_post)
            echo "finished clean of targets ${TARGETS}"
        ;;

        fetch_pre)
            echo "starting fetch of package ${TARGET}${COUNTS}"
        ;;

        fetch_post)
            echo "finished fetch of package ${TARGET}${COUNTS}"
        ;;

        fetch_all_pre)
            echo "starting fetch of targets ${TARGETS}"
        ;;

        fetch_all_post)
            echo "finished fetch of targets ${TARGETS}"
        ;;

        sync_pre)
            echo "starting sync of repository ${TARGET}${COUNTS}"
        ;;

        sync_post)
            echo "finished sync of repository ${TARGET}${COUNTS}"
        ;;

        *)
            echo "unknown hook ${HOOK}"
    esac
) >> "${PALUDIS_LOG_DIRECTORY:-${ROOT}/var/log}/paludis.log"

true
