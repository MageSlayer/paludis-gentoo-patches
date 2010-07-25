#!/usr/bin/env bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2008, 2010 Ciaran McCreesh
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

make_binary_tarball()
{
    local imagedir="${1}" envfile="${2}" bindistfile="${3}"
    local tmpdir="${PALUDIS_TMPDIR}/pbin-$(basename "${bindistfile}" )"

    echo tar cvf "${bindistfile}".tar --files-from /dev/null 1>&2
    tar cvf "${bindistfile}".tar --files-from /dev/null || die "making tarball failed"

    echo find "${imagedir}" -mindepth 1 -maxdepth 1 -printf '%f\0' \| \
            xargs -0 tar rpvf "${bindistfile}".tar -C "${imagedir}" 1>&2
    find "${imagedir}" -mindepth 1 -maxdepth 1 -printf '%f\0' | \
            xargs -0 tar rpvf "${bindistfile}".tar -C "${imagedir}" \
            || die "adding image to tarball failed"

    echo rm -fr "${tmpdir}" 1>&2
    rm -fr "${tmpdir}"

    echo mkdir "${tmpdir}" 1>&2
    mkdir "${tmpdir}" || die "mkdir ${tmpdir} failed"

    echo mkdir "${tmpdir}"/PBIN 1>&2
    mkdir "${tmpdir}"/PBIN 1>&2

    echo bzip2 \< "${envfile}" \> "${tmpdir}/PBIN/environment.bz2" 1>&2
    bzip2 < "${envfile}" > "${tmpdir}/PBIN/environment.bz2"

    echo "tar rvf "${bindistfile}".tar -C "${tmpdir}" 'PBIN'" 1>&2
    tar rvf "${bindistfile}".tar -C "${tmpdir}" 'PBIN' || die "adding env to tarball failed"

    echo rm -fr "${tmpdir}" 1>&2
    rm -fr "${tmpdir}"

    echo bzip2 "${bindistfile}".tar 1>&2
    bzip2 "${bindistfile}".tar || die "compressing tarball failed"
}

make_binary_ebuild()
{
    local ebuildfile="${1}" bin_uri="${2}" binary_keywords="${3}"

    export GENERATED_TIME=$(date +%s )
    export GENERATED_USING=${PKGMANAGER}
    export BINARY_URI="${bin_uri}"
    export EAPI="pbin-1+${EAPI}"
    export GENERATED_FROM_REPOSITORY="${REPOSITORY}"
    echo export ${PALUDIS_BINARY_KEYWORDS_VARIABLE}="${binary_keywords}" 1>&2
    export ${PALUDIS_BINARY_KEYWORDS_VARIABLE}="${binary_keywords}"

    echo "# Created by $PKGMANAGER on $(date )" > ${ebuildfile} || die "Couldn't write ${ebuildfile}"

    local p
    for p in ${PALUDIS_BINARY_FROM_ENV_VARIABLES} ; do
        set -o noglob
        local k=$(paludis_rewrite_var BINARY "${p}" "${!p}" )
        k=${k//\\/\\\\}
        k=${k//\"/\\\"}
        # {"} fix vim syntax highlighting
        k=${k//\$/\\\$}
        echo "${p}=\""${k}"\"" >> "${ebuildfile}"
        set +o noglob
    done

    sed -e 's,^,        ,' < ${ebuildfile}
}

