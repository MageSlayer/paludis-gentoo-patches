#!/usr/bin/env bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2006, 2007, 2008, 2009 Ciaran McCreesh
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

builtin_init()
{
    local a

    for a in ${PALUDIS_DIRECTORY_VARIABLES} ; do
        a=${a#build:}
        [[ -d "${!a}" ]] || die "\$${a} (\"${!a}\") not a directory"
    done

    for a in ${PALUDIS_DIRECTORY_IF_EXISTS_VARIABLES} ; do
        a=${a#build:}
        [[ -e "${!a}" ]] && [[ ! -d "${!a}" ]] && \
            die "\$${a} (\"${!a}\") exists but is not a directory"
    done

    for a in PALUDIS_TMPDIR ; do
        if ! [[ -d "${!a}" ]] ; then
            PALUDIS_EXTRA_DIE_MESSAGE="
!!! '${!a}' should be a directory, but does not exist. For,
!!! security reasons, Paludis will not try to create this directory
!!! automatically. Please create it by hand and give it appropriate
!!! permissions. Typically you should use:
!!!     mkdir ${!a}
!!!     chgrp ${PALUDIS_REDUCED_GID} ${!a}
!!!     chmod g+rwx ${!a}
!!! although other values may be more appropriate for your system.
"
            die "\$${a} (\"${!a}\") not a directory"
        fi
    done

    if [[ -e "${PALUDIS_PACKAGE_BUILDDIR}" ]] ; then
        if type -p chflags &>/dev/null; then
            chflags -R 0 "${PALUDIS_PACKAGE_BUILDDIR}" || die "Couldn't remove flags from workdir"
        fi
        rm -fr "${PALUDIS_PACKAGE_BUILDDIR}" || die "Couldn't remove previous work"
    fi

    export WORKDIR="${PALUDIS_PACKAGE_BUILDDIR}/work"
    mkdir -p "${WORKDIR}" || die "Couldn't create \$WORKDIR (\"${WORKDIR}\")"
    declare -r WORKDIR="${WORKDIR}"

    export T="${PALUDIS_PACKAGE_BUILDDIR}/temp/"
    mkdir -p "${T}" || die "Couldn't create \$T (\"${T}\")"
    declare -r T="${T}"
    export HOME="${T}"
    export TMPDIR="${T}"

    export D="${PALUDIS_PACKAGE_BUILDDIR}/image/"
    export D="${D//+(\/)//}"
    mkdir -p "${D}" || die "Couldn't create \$D (\"${D}\")"
    declare -r D="${D}"

    [[ -n "${PALUDIS_PREFIX_IMAGE_VAR}" ]] && export $PALUDIS_PREFIX_IMAGE_VAR=${D}

    IMAGE="${D}"
    declare -r IMAGE="${IMAGE}"

    export S="${WORKDIR}/${P}"

    ebuild_load_em_up_dan

    for a in PALUDIS_NON_EMPTY_VARIABLES ${PALUDIS_NON_EMPTY_VARIABLES} ; do
        a=${a#build:}
        [[ -z "${!a}" ]] && die "\$${a} unset or empty"
        declare -r ${a}="${!a}"
    done
}

ebuild_f_init()
{
    if hasq "init" ${SKIP_FUNCTIONS} ; then
        ebuild_section "Skipping builtin_init (SKIP_FUNCTIONS)"
    else
        ebuild_section "Starting builtin_init"
        builtin_init
        ebuild_section "Done builtin_init"
    fi
}

