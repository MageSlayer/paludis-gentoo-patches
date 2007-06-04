#!/bin/bash
# vim: set sw=4 sts=4 et :

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

unalias -a
set +C
unset GZIP BZIP BZIP2 CDPATH GREP_OPTIONS GREP_COLOR GLOBIGNORE
eval unset LANG ${!LC_*}

if [[ -z "${PALUDIS_DO_NOTHING_SANDBOXY}" ]] ; then
    export SANDBOX_WRITE="${SANDBOX_WRITE}/dev/shm:/dev/stdout:/dev/stderr:/dev/null:/dev/tty:/dev/pts"
    export SANDBOX_WRITE="${SANDBOX_WRITE}:/proc/self/attr:/proc/self/task:/selinux/context"
    export SANDBOX_ON="1"
    export SANDBOX_BASHRC="/dev/null"
    unset BASH_ENV
fi

shopt -s expand_aliases
shopt -s extglob

case "${1}" in
    specification)
        gem specification "${2}" -v "${3}"
        exit $?
        ;;

    install)
        gem install "${2}" -v "${3}"
        exit $?
        ;;

    uninstall)
        gem uninstall "${2}" -v "${3}"  -i -x
        exit $?
        ;;

    *)
        echo "Unknown action ${1}" 1>&2
        exit 1
        ;;
esac
