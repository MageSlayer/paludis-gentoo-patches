#!/usr/bin/env bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2006, 2007, 2010 Ciaran McCreesh
#
# Based in part upon ebuild.sh from Portage, which is Copyright 1995-2005
# Gentoo Foundation and distributed under the terms of the GNU General
# Public License v2.
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

ebuild_f_metadata()
{
    local key

    for a in ${PALUDIS_EBUILD_MUST_NOT_SET_VARIABLES} ; do
        [[ ${!a+set} != set ]] || die "\$${a} must not be set"
    done

    for key in ${EBUILD_METADATA_VARIABLES} ; do
        set -o noglob
        local k=${!key}
        k=${k//\\/\\\\}
        k=${k//\"/\\\"}
        k=${k//\$/\\\$}
        echo "${key}=\""${k}"\"" 1>&${PALUDIS_METADATA_FD:-1}
        set +o noglob
    done
}

