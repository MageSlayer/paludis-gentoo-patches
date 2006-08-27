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

mkdir -p "${ROOT}/var/log"

(
    echo -n "$(date +%s ): "

    case "${HOOK}" in
        install_pre)
            echo "starting install of package ${TARGET}"
        ;;

        install_post)
            echo "finished install of package ${TARGET}"
        ;;

        install_all_pre)
            echo "starting install of targets ${TARGETS}"
        ;;

        install_all_post)
            echo "finished install of targets ${TARGETS}"
        ;;

        uninstall_pre)
            echo "starting uninstall of package ${TARGET}"
        ;;

        uninstall_post)
            echo "finished uninstall of package ${TARGET}"
        ;;

        uninstall_all_pre)
            echo "starting uninstall of targets ${TARGETS}"
        ;;

        uninstall_all_post)
            echo "finished uninstall of targets ${TARGETS}"
        ;;

        fetch_pre)
            echo "starting fetch of package ${TARGET}"
        ;;

        fetch_post)
            echo "finished fetch of package ${TARGET}"
        ;;

        fetch_all_pre)
            echo "starting fetch of targets ${TARGETS}"
        ;;

        fetch_all_post)
            echo "finished fetch of targets ${TARGETS}"
        ;;

        sync_pre)
            echo "starting sync of repository ${TARGET}"
        ;;

        sync_post)
            echo "finished sync of repository ${TARGET}"
        ;;

        *)
            echo "unknown hook ${HOOK}"
    esac
) >> ${ROOT}/var/log/paludis.log

