#!/bin/bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2006, 2007, 2008 Ciaran McCreesh
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

default_src_install()
{
    local done_docs old_set f d p doc e
    if [[ -f Makefile ]] || [[ -f makefile ]] || [[ -f GNUmakefile ]] ; then
        if make -j1 -n install ; then
            echo "Found a makefile, using the install target" 
            emake -j1 DESTDIR="${D}" install || die "default emake install failed";
        else
            die "default emake install located a makefile but no install target"
        fi
    else
        echo "No makefile found, not using emake install"
    fi
    done_docs=
    old_set=$(shopt | grep 'nocaseglob[[:space:]]*on')
    shopt -s nocaseglob
    for d in '' ${DEFAULT_SRC_INSTALL_EXTRA_SUBDIRS[@]} ; do
        if [[ -n ${d} ]]; then
            [[ -d ${d} ]] || die "${d} is not a dir"
            pushd "${d}" > /dev/null || die "Failed to enter ${d}"
            local docdesttree="${DOCDESTTREE}"
            docinto "${d}"
        fi
        for f in README Change{,s,Log} AUTHORS NEWS TODO ABOUT THANKS {KNOWN_,}BUGS SUBMITTING \
            HACKING FAQ CREDITS PKG-INFO HISTORY PACKAGING MAINTAINER{,S} CONTRIBUT{E,OR,ORS} RELEASE \
            ANNOUNCE PORTING NOTES PROBLEMS NOTICE ${DEFAULT_SRC_INSTALL_EXTRA_DOCS[@]}; do
            for p in ${DEFAULT_SRC_INSTALL_EXTRA_PREFIXES[@]} '' ; do
                for doc in *([[:digit:]])${p}${f}{,+([._-])*} ; do
                    if [[ -s "${doc}" ]] ; then
                        for e in ${DEFAULT_SRC_INSTALL_EXCLUDE[@]} ; do
                            [[ ${doc} == ${e} ]] && continue 2
                        done
                        done_docs="${done_docs} ${d%/}${d:+/}${doc}"
                        dodoc "${doc}" || die "dodoc ${d%/}${d:+/}${doc} failed"
                    fi
                done
            done
        done
        if [[ -n ${d} ]]; then
            docinto "${docdesttree}"
            popd > /dev/null || die "Failed to leave ${d}"
        fi
    done
    if [[ -n "${done_docs}" ]] ; then
        echo "Installed docs ${done_docs# }"
    else
        echo "Didn't find any docs to install"
    fi
    [[ -n ${old_set} ]] || shopt -u nocaseglob
}

src_install()
{
    default "$@"
}

exheres_internal_install()
{
    if [[ -d "${S}" ]] ; then
        cd "${S}" || die "cd to \${S} (\"${S}\") failed"
    elif [[ -d "${WORKDIR}" ]] ; then
        cd "${WORKDIR}" || die "cd to \${WORKDIR} (\"${WORKDIR}\") failed"
    fi

    if hasq "install" ${SKIP_FUNCTIONS} ; then
        ebuild_section "Skipping src_install (SKIP_FUNCTIONS)"
    else
        ebuild_section "Starting src_install"
        src_install
        ebuild_section "Done src_install"
    fi
}
