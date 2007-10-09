#!/bin/bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

unalias -a
set +C
unset GZIP BZIP BZIP2 CDPATH GREP_OPTIONS GREP_COLOR GLOBIGNORE
eval unset LANG ${!LC_*}

# The list below should include all variables from all EAPIs
EBUILD_METADATA_VARIABLES="DEPEND RDEPEND PDEPEND IUSE SRC_URI RESTRICT \
    LICENSE KEYWORDS INHERITED PROVIDE HOMEPAGE DESCRIPTION DEPENDENCIES \
    E_IUSE E_DEPEND E_RDEPEND E_PDEPEND E_KEYWORDS PLATFORMS E_PLATFORMS \
    MYOPTIONS E_MYOPTIONS"
unset -v ${EBUILD_METADATA_VARIABLES} ${PALUDIS_EBUILD_MUST_NOT_SET_VARIABLES}
# These can be set by C++
EBUILD_METADATA_VARIABLES="${EBUILD_METADATA_VARIABLES} SLOT EAPI"

if [[ -z "${PALUDIS_DO_NOTHING_SANDBOXY}" ]] ; then
    export SANDBOX_PREDICT="${SANDBOX_PREDICT+${SANDBOX_PREDICT}:}"
    export SANDBOX_PREDICT="${SANDBOX_PREDICT}/proc/self/maps:/dev/console:/dev/random"
    export SANDBOX_WRITE="${SANDBOX_WRITE+${SANDBOX_WRITE}:}"
    export SANDBOX_WRITE="${SANDBOX_WRITE}/dev/shm:/dev/stdout:/dev/stderr:/dev/null:/dev/tty:/dev/pts"
    export SANDBOX_WRITE="${SANDBOX_WRITE}:${PALUDIS_TMPDIR}:/var/cache"
    export SANDBOX_WRITE="${SANDBOX_WRITE}:/proc/self/attr:/proc/self/task:/selinux/context"
    [[ -n "${CCACHE_DIR}" ]] && export SANDBOX_WRITE="${SANDBOX_WRITE}:${CCACHE_DIR}"
    export SANDBOX_ON="1"
    export SANDBOX_BASHRC="/dev/null"
    unset BASH_ENV
fi

shopt -s expand_aliases
shopt -s extglob

export EBUILD_PROGRAM_NAME="$0"

if [[ -n "${PALUDIS_EBUILD_DIR_FALLBACK}" ]] ; then
    export PATH="${PALUDIS_EBUILD_DIR_FALLBACK}/utils:${PATH}"
fi
export PATH="${PALUDIS_EBUILD_DIR}/utils:${PATH}"
for p in ${PALUDIS_UTILITY_PATH_SUFFIXES} ; do
    export PATH="${PALUDIS_EBUILD_DIR}/utils/${p}:${PATH}"
done

# Force a few more things into PATH, since some users have crazy setups.
# See ticket:374.
export PATH="${PATH}:/bin:/sbin:/usr/bin:/usr/sbin"

EBUILD_MODULES_DIR=$(canonicalise $(dirname $0 ) )
if ! [[ -d ${EBUILD_MODULES_DIR} ]] ; then
    echo "${EBUILD_MODULES_DIR} is not a directory" 1>&2
    exit 123
fi
export PALUDIS_EBUILD_MODULES_DIR="${EBUILD_MODULES_DIR}"

for p in ${PALUDIS_EBUILD_MODULE_SUFFIXES}; do
    EBUILD_MODULES_DIRS="${EBUILD_MODULES_DIRS} ${EBUILD_MODULES_DIR}/${p}"
done
EBUILD_MODULES_DIRS="${EBUILD_MODULES_DIRS} ${EBUILD_MODULES_DIR}"

ebuild_load_module()
{
    for d in ${EBUILD_MODULES_DIRS}; do
        if [[ -f "${d}/${1}.bash" ]]; then
            if ! source "${d}/${1}.bash"; then
                type die &>/dev/null && die "Error loading module ${1}"
                echo "Error loading module ${1}" 1>&2
                exit 123
            fi
            return
        fi
    done
    type die &>/dev/null && die "Couldn't find module ${1}"
    echo "Couldn't find module ${1}" 1>&2
    exit 123
}

ebuild_load_module die_functions
ebuild_load_module echo_functions
ebuild_load_module kernel_functions
ebuild_load_module sandbox
ebuild_load_module portage_stubs
ebuild_load_module list_functions
ebuild_load_module multilib_functions
ebuild_load_module install_functions
ebuild_load_module build_functions
ebuild_load_module eclass_functions
ebuild_load_module exlib_functions
ebuild_load_module work_around_broken_utilities

export PALUDIS_HOME="$(canonicalise ${PALUDIS_HOME:-${HOME}} )"

ebuild_source_profile()
{
    if [[ -f ${1}/parent ]] ; then
        while read line ; do
            grep --silent '^[[:space:]]*#' <<<"${line}" && continue
            grep --silent '[^[:space:]]' <<<"${line}" || continue
            ebuild_source_profile $(canonicalise ${1}/${line} )
        done <${1}/parent
    fi

    local old_set=$-
    set -a

    if [[ -f ${1}/make.defaults ]] ; then
        source ${1}/make.defaults || die "Couldn't source ${1}/make.defaults"
    fi

    if [[ -f ${1}/bashrc ]] ; then
        source ${1}/bashrc || die "Couldn't source ${1}/bashrc"
    fi

    [[ "${old_set}" == *a* ]] || set +a
}

export CONFIG_PROTECT="${PALUDIS_CONFIG_PROTECT}"
export CONFIG_PROTECT_MASK="${PALUDIS_CONFIG_PROTECT_MASK}"
save_vars="$(eval echo ${PALUDIS_SAVE_VARIABLES} )"
save_base_vars="$(eval echo ${PALUDIS_SAVE_BASE_VARIABLES} )"
save_unmodifiable_vars="$(eval echo ${PALUDIS_SAVE_UNMODIFIABLE_VARIABLES} )"
check_save_vars="${save_vars}"
check_base_vars="${save_base_vars}"
check_unmodifiable_vars="${save_unmodifiable_vars}"

for var in ${save_vars} ${default_save_vars} ${save_base_vars} ${save_unmodifiable_vars} ; do
    ebuild_notice "debug" "Saving ${var}=${!var}"
    eval "export save_var_${var}='${!var}'"
done

if [[ -n "${PALUDIS_PROFILE_DIRS}" ]] ; then
    for var in ${PALUDIS_PROFILE_DIRS} ; do
        ebuild_source_profile $(canonicalise "${var}")
    done
elif [[ -n "${PALUDIS_PROFILE_DIR}" ]] ; then
    ebuild_source_profile $(canonicalise "${PALUDIS_PROFILE_DIR}")
fi

unset ${save_vars} ${save_base_vars}

for f in ${PALUDIS_BASHRC_FILES} ; do
    if [[ -f ${f} ]] ; then
        ebuild_notice "debug" "Loading bashrc file ${f}"
        old_set=$-
        set -a
        source ${f}
        [[ "${old_set}" == *a* ]] || set +a
    else
        ebuild_notice "debug" "Skipping bashrc file ${f}"
    fi

    for var in ${check_save_vars} ; do
        if [[ -n ${!var} ]] ; then
            die "${f} attempted to set \$${var}, which must not be set in bashrc."
        fi
    done

    for var in ${check_save_unmodifiable_vars} ; do
        s_var=save_var_${var}
        if [[ "${!s_var}" != "${!var}" ]] ; then
            die "${f} attempted to modify \$${var}, which must not be modified in bashrc."
        fi
    done
done

for var in ${save_vars} ; do
    eval "export ${var}=\${save_var_${var}}"
done

for var in ${save_base_vars} ; do
    eval "export ${var}=\"\${save_var_${var}} \$$(echo ${var})\""
done

[[ -z "${CBUILD}" ]] && export CBUILD="${CHOST}"
export REAL_CHOST="${CHOST}"

ebuild_scrub_environment()
{
    local filters=(
        -e '/^\(EU\|PP\|U\)ID=/d'
        -e '/^BASH_\(ARGC\|ARGV\|LINENO\|SOURCE\|VERSINFO\|REMATCH\)=/d'
        -e '/^BASH_COMPLETION\(_DIR\)\?=/d'
        -e '/^PALUDIS_SOURCE_MERGED_VARIABLES=/d'
        -e '/^bash[0-9]\+[a-z]\?=/d'
        -e '/^\(FUNCNAME\|GROUPS\|SHELLOPTS\)=/d'
        -e '/^\(declare -x \|export \)\?SANDBOX_ACTIVE=/d'
    )

    sed -i "${filters[@]}" "${1}"

    (
        source "${1}" || exit 1

        unset -f diefunc perform_hook inherit builtin_loadenv builtin_saveenv

        unset -v PATH ROOTPATH T PALUDIS_TMPDIR PALUDIS_EBUILD_LOG_LEVEL
        unset -v PORTDIR FILESDIR ECLASSDIR DISTDIR PALUDIS_EBUILD_DIR
        unset -v PALUDIS_EXTRA_DIE_MESSAGE PALUDIS_COMMAND PALUDIS_CLIENT
        unset -v PALUDIS_LOADSAVEENV_DIR SKIP_FUNCTIONS PALUDIS_DO_NOTHING_SANDBOXY
        unset -v FETCHEDDIR REPODIR

        unset -v ${!PALUDIS_CMDLINE_*} PALUDIS_OPTIONS
        unset -v ${!CONTRARIUS_CMDLINE_*} CONTRARIUS_OPTIONS
        unset -v ${!GTKPALUDIS_CMDLINE_*} GTKPALUDIS_OPTIONS
        unset -v ${!ADJUTRIX_CMDLINE_*} ADJUTRIX_OPTIONS
        unset -v ${!QUALUDIS_CMDLINE_*} QUALUDIS_OPTIONS
        unset -v ${!RECONCILIO_CMDLINE_*} RECONCILIO_OPTIONS

        unset -v PALUDIS_HOME PALUDIS_PID EBUILD_KILL_PID ROOT
        unset -v CATEGORY PN PV P PVR PF ${!LD_*}

        unset -v ebuild EBUILD
        for v in ${PALUDIS_SOURCE_MERGED_VARIABLES} ; do
            e_v=E_${v}
            unset -v ${e_v}
        done

        for v in ${!SANDBOX*}; do
            [[ "${v}" == SANDBOX_ACTIVE ]] || unset "${v}"
        done

        for v in ${!BASH_*}; do
            case "${v#BASH_}" in
                ARGC|ARGV|LINENO|SOURCE|VERSINFO) ;;
                *) unset -v "${v}"
            esac
        done

        set >"${1}"
        export -p >>"${1}"
    ) || return $?

    sed -i \
        -e 's:^declare -rx:declare -x:' \
        -e 's:^declare -x :export :' \
        "${filters[@]}" "${1}"
}

ebuild_load_environment()
{
    if [[ -n "${PALUDIS_LOAD_ENVIRONMENT}" ]] ; then
        [[ -d ${PALUDIS_TMPDIR} ]] \
            || die "You need to create PALUDIS_TMPDIR (${PALUDIS_TMPDIR})."

        local save_PALUDIS_EXTRA_DIE_MESSAGE="${PALUDIS_EXTRA_DIE_MESSAGE}"
        export PALUDIS_EXTRA_DIE_MESSAGE="
!!! Could not extract the saved environment file. This is usually
!!! caused by a broken environment.bz2 that was generated by an old
!!! Portage version. The file that needs repairing is:
!!!     ${PALUDIS_LOAD_ENVIRONMENT}
!!! Try copying this file, bunzip2ing it and sourcing it using a new
!!! bash shell (do not continue to use said shell afterwards). You
!!! should get an error that give you a rough idea of where the
!!! problem lies.
"

        if [[ "${PALUDIS_LOAD_ENVIRONMENT%.bz2}" != "${PALUDIS_LOAD_ENVIRONMENT}" ]] ; then
            echo bunzip2 \< "${PALUDIS_LOAD_ENVIRONMENT}" \> ${PALUDIS_TMPDIR}/environment-${CATEGORY}-${PF}-$$ 1>&2
            bunzip2 < "${PALUDIS_LOAD_ENVIRONMENT}" > ${PALUDIS_TMPDIR}/environment-${CATEGORY}-${PF}-$$ \
                || die "Can't extract ${PALUDIS_LOAD_ENVIRONMENT}"
        else
            echo cp "${PALUDIS_LOAD_ENVIRONMENT}" "${PALUDIS_TMPDIR}/environment-${CATEGORY}-${PF}-$$" 1>&2
            cp "${PALUDIS_LOAD_ENVIRONMENT}" "${PALUDIS_TMPDIR}/environment-${CATEGORY}-${PF}-$$" \
                || die "Can't copy ${PALUDIS_LOAD_ENVIRONMENT}"
        fi

        echo ebuild_scrub_environment "${PALUDIS_TMPDIR}/environment-${CATEGORY}-${PF}-$$" 1>&2
        ebuild_scrub_environment "${PALUDIS_TMPDIR}/environment-${CATEGORY}-${PF}-$$" \
            || die "Can't load saved environment for cleaning"

        echo source "${PALUDIS_TMPDIR}/environment-${CATEGORY}-${PF}-$$" 1>&2
        source "${PALUDIS_TMPDIR}/environment-${CATEGORY}-${PF}-$$" \
            || die "Can't load saved environment"

        export PALUDIS_EXTRA_DIE_MESSAGE="${save_PALUDIS_EXTRA_DIE_MESSAGE}"

        echo rm "${PALUDIS_TMPDIR}/environment-${CATEGORY}-${PF}-$$" 1>&2
        rm "${PALUDIS_TMPDIR}/environment-${CATEGORY}-${PF}-$$"
    fi
}

ebuild_load_ebuild()
{
    export EBUILD="${1}"
    unset ${SOURCE_MERGED_VARIABLES}

    local v e_v
    for v in ${PALUDIS_MUST_NOT_CHANGE_VARIABLES} ; do
        e_v=saved_${v}
        local ${e_v}="${!v}"
    done

    [[ -f "${1}" ]] || die "Ebuild '${1}' is not a file"
    source ${1} || die "Error sourcing ebuild '${1}'"

    if [[ -n "${PALUDIS_RDEPEND_DEFAULTS_TO_DEPEND}" ]] ; then
        [[ ${RDEPEND-unset} == "unset" ]] && RDEPEND="${DEPEND}"
    fi

    for v in ${PALUDIS_SOURCE_MERGED_VARIABLES} ; do
        e_v=E_${v}
        export -n ${v}="${!v} ${!e_v}"
    done

    [[ ${EAPI-unset} == "unset" ]] && EAPI="0"

    for v in ${PALUDIS_MUST_NOT_CHANGE_VARIABLES} ; do
        s_v="saved_${v}"
        if [[ -n "${!s_v}" ]] && [[ "${!v}" != "${!s_v}" ]] ; then
            ebuild_notice "qa" \
                "Ebuild ${1} illegally tried to change ${v} from '${!s_v}' to '${!v}'"
            export ${v}="${!s_v}"
        fi
    done
}

perform_hook()
{
    export HOOK=${1}
    ebuild_notice "debug" "Starting hook '${HOOK}'"

    local old_sandbox_on="${SANDBOX_ON}"
    [[ -z "${PALUDIS_DO_NOTHING_SANDBOXY}" ]] && export SANDBOX_ON="0"

    local hook_dir
    for hook_dir in ${PALUDIS_HOOK_DIRS} ; do
        [[ -d "${hook_dir}/${HOOK}" ]] || continue
        local hook_file
        for hook_file in "${hook_dir}/${HOOK}/"*.bash ; do
            [[ -e "${hook_file}" ]] || continue
            ebuild_notice "debug" "Starting hook script '${hook_file}' for '${HOOK}'"
            if ! ( source "${hook_file}" ) ; then
                ebuild_notice "warning" "Hook '${hook_file}' returned failure"
            else
                ebuild_notice "debug" "Hook '${hook_file}' returned success"
            fi
        done
    done

    [[ -z "${PALUDIS_DO_NOTHING_SANDBOXY}" ]] && export SANDBOX_ON="${old_sandbox_on}"
    true
}

ebuild_main()
{
    if ! [[ -e /proc/self ]] && [[ "$(uname -s)" == Linux ]] ; then
        ebuild_notice "warning" "/proc appears to be unmounted or unreadable."
        ebuild_notice "warning" "This will cause problems."
    fi

    local action ebuild="$1"
    shift

    ebuild_notice "debug" "Using ebuild '${ebuild}', EAPI before source is '${EAPI}'"

    if [[ ${#@} -ge 2 ]] ; then
        ebuild_section "Running ebuild phases $@ as $(id -un ):$(id -gn )..."
    elif [[ ${1} != variable ]] && [[ ${1} != metadata ]] && [[ ${1} != pretend ]] ; then
        ebuild_section "Running ebuild phase $@ as $(id -un ):$(id -gn )..."
    fi

    for action in $@ ; do
        case ${action} in
            metadata|variable|init|prepare|merge|unmerge|tidyup|\
                    strip|loadenv|saveenv|initbin|unpackbin|infovars)
                ebuild_load_module builtin_${action}
            ;;

            unpack|compile|install|test)
                ebuild_load_module src_${action}
            ;;

            setup|config|nofetch|preinst|postinst|prerm|postrm|pretend|info)
                ebuild_load_module pkg_${action}
            ;;

            *)
                ebuild_load_module usage_error
                ebuild_f_usage_error "Unknown action '${action}'"
                exit 1
            ;;
        esac
    done

    if [[ $1 == metadata ]] || [[ $1 == variable ]] ; then
        export EBUILD_PHASE="${1}"
        perform_hook ebuild_${action}_pre
        if [[ $1 != variable ]] || [[ -n "${ebuild}" ]] ; then
            for f in cut tr date ; do
                eval "export ebuild_real_${f}=\"$(which $f )\""
                eval "${f}() { ebuild_notice qa 'global scope ${f}' ; $(which $f ) \"\$@\" ; }"
            done
            PATH="" ebuild_load_ebuild "${ebuild}"
        fi
        if ! ebuild_f_${1} ; then
            perform_hook ebuild_${action}_fail
            die "${1} failed"
        fi
        perform_hook ebuild_${action}_post
    else
        ebuild_load_environment
        if [[ "${ebuild}" != "-" ]] ; then
            ebuild_load_ebuild "${ebuild}"
        fi
        for action in $@ ; do
            export EBUILD_PHASE="${action}"
            perform_hook ebuild_${action}_pre
            if ! ebuild_f_${action} ; then
                perform_hook ebuild_${action}_fail
                die "${action} failed"
            fi
            if [[ ${action} == "init" ]] ; then
                ebuild_load_ebuild "${ebuild}"
            fi
            perform_hook ebuild_${action}_post
        done
    fi

    if [[ ${#@} -ge 2 ]] ; then
        ebuild_section "Completed ebuild phases $@"
    elif [[ ${1} != variable ]] && [[ ${1} != metadata ]] && [[ ${1} != pretend ]] ; then
        ebuild_section "Completed ebuild phase $@"
    fi
}

ebuild_main "$@"

