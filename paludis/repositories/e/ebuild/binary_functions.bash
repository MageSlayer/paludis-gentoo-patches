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

