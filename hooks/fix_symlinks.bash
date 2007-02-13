#!/bin/bash
# vim: set et sw=4 sts=4 :

# Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

shopt -s extglob

image=$(tr -s / <<<"${IMAGE}" )

case "${HOOK}" in

    merger_check_sym_post)
        target=$(readlink ${INSTALL_SOURCE} | tr -s / )
        [[ "${target#${image}}" == "${target}" ]] && exit 0

        ewarn "Bad symlink ${INSTALL_SOURCE} -> $(readlink ${INSTALL_SOURCE} ) will be rewritten"
    ;;

    merger_install_sym_post)
        target=$(readlink ${INSTALL_DESTINATION} | tr -s / )
        [[ "${target#${image}}" == "${target}" ]] && exit 0

        new_target=${target#${image}}
        new_target=/${new_target##+(/)}
        ewarn "Relinking bad symlink ${INSTALL_DESTINATION} -> $(readlink \
            ${INSTALL_DESTINATION} ) to ${new_target}"
        echo rm -f "${INSTALL_DESTINATION}" 1>&2
        rm -f "${INSTALL_DESTINATION}"
        echo ln -s "${new_target}" "${INSTALL_DESTINATION}" 1>&2
        ln -s "${new_target}" "${INSTALL_DESTINATION}"
    ;;

    *)
    ewarn "$0: Don't know how to respond to HOOK ${HOOK}"
    ;;

esac

true

