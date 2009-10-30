#!/usr/bin/env bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2006, 2007, 2008, 2009 Ciaran McCreesh
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

[[ -n "${PALUDIS_TRACE}" ]] && set -x

ebuild_sanitise_envvars()
{
    local p

    # Force a few more things into PATH, since some users have crazy setups.
    # See ticket:374.
    export PATH="/bin:/sbin:/usr/bin:/usr/sbin${PATH:+:${PATH}}"

    # Automake likes to scatter our utilities over two directories.
    if [[ -n "${PALUDIS_EBUILD_DIR_FALLBACK}" ]] ; then
        export PATH="${PALUDIS_EBUILD_DIR_FALLBACK}/utils:${PATH}"
        for p in ${PALUDIS_UTILITY_PATH_SUFFIXES} ; do
            export PATH="${PALUDIS_EBUILD_DIR_FALLBACK}/utils/${p}:${PATH}"
        done
    fi

    export PATH="${PALUDIS_EBUILD_DIR}/utils:${PATH}"
    for p in ${PALUDIS_UTILITY_PATH_SUFFIXES} ; do
        export PATH="${PALUDIS_EBUILD_DIR}/utils/${p}:${PATH}"
    done

    unset GZIP BZIP BZIP2 CDPATH GREP_OPTIONS GREP_COLOR GLOBIGNORE
    unset LANG ${!LC_*}
    export LC_ALL=C
}
ebuild_sanitise_envvars

# The list below should include all variables from all EAPIs, along with any
# fancy fake variables
EBUILD_METADATA_VARIABLES="DEPEND RDEPEND PDEPEND IUSE SRC_URI DOWNLOADS RESTRICT \
    LICENSE LICENCES KEYWORDS INHERITED PROVIDE HOMEPAGE DESCRIPTION DEPENDENCIES \
    E_IUSE E_DEPEND E_RDEPEND E_PDEPEND PLATFORMS \
    MYOPTIONS E_MYOPTIONS E_DEPENDENCIES BINARY_KEYWORDS BINARY_URI \
    GENERATED_USING GENERATED_TIME BINARY_PLATFORMS REMOTE_IDS \
    SUMMARY BUGS_TO UPSTREAM_DOCUMENTATION UPSTREAM_CHANGELOG \
    UPSTREAM_RELEASE_NOTES PROPERTIES PALUDIS_DECLARED_FUNCTIONS SLOT EAPI OPTIONS USE \
    PALUDIS_EBUILD_RDEPEND_WAS_SET PALUDIS_EBUILD_DEPEND"
EBUILD_METADATA_VARIABLES_FROM_CPLUSPLUS="SLOT EAPI OPTIONS USE"

if [[ -z "${PALUDIS_DO_NOTHING_SANDBOXY}" ]] ; then
    export SANDBOX_PREDICT="${SANDBOX_PREDICT+${SANDBOX_PREDICT}:}"
    export SANDBOX_PREDICT="${SANDBOX_PREDICT}/proc/self/maps:/dev/console:/dev/random"
    export SANDBOX_WRITE="${SANDBOX_WRITE+${SANDBOX_WRITE}:}"
    export SANDBOX_WRITE="${SANDBOX_WRITE}/dev/shm:/dev/stdout:/dev/stderr:/dev/null:/dev/tty:/dev/pts"
    export SANDBOX_WRITE="${SANDBOX_WRITE}:${PALUDIS_TMPDIR}:/var/cache"
    export SANDBOX_WRITE="${SANDBOX_WRITE}:/proc/self/attr:/proc/self/task:/selinux/context"
    export SANDBOX_ON="1"
    export SANDBOX_BASHRC="/dev/null"
    unset BASH_ENV
fi

shopt -s expand_aliases
shopt -s extglob

export ROOT="${ROOT%+(/)}/"

export EBUILD_PROGRAM_NAME="$0"

EBUILD_MODULES_DIR=$(canonicalise $(dirname $0 ) )
if ! [[ -d ${EBUILD_MODULES_DIR} ]] ; then
    echo "${EBUILD_MODULES_DIR} is not a directory" 1>&2
    exit 123
fi

# Upgrade from back when 0/ wasn't its own dir.
[[ -z "${PALUDIS_EBUILD_MODULE_SUFFIXES}" ]] && PALUDIS_EBUILD_MODULE_SUFFIXES=0
for p in ${PALUDIS_EBUILD_MODULE_SUFFIXES}; do
    EBUILD_MODULES_DIRS="${EBUILD_MODULES_DIRS} ${EBUILD_MODULES_DIR}/${p}"
done
for p in ${PALUDIS_EXTRA_EBUILD_MODULES_DIRS} ; do
    EBUILD_MODULES_DIRS="${EBUILD_MODULES_DIRS} ${p}"
done
EBUILD_MODULES_DIRS="${EBUILD_MODULES_DIRS} ${EBUILD_MODULES_DIR}"

export PALUDIS_EBUILD_MODULES_DIR="${EBUILD_MODULES_DIR}"

export EBUILD_KILL_PID=$$
declare -r EBUILD_KILL_PID

ebuild_load_module()
{
    local older= t= d= save_excl= excl_v=
    if [[ "${1}" == "--older" ]] ; then
        shift
        older=true
        excl_v="EBUILD_MODULES_DIRS_EXCLUDE_${1}"
        save_excl="${!excl_v}"
    fi

    for d in ${EBUILD_MODULES_DIRS}; do
        local dx= x=
        if [[ -n "${older}" ]] ; then
            for dx in ${!excl_v} ; do
                [[ "${dx}" == "${d}" ]] && x=true
            done
        fi
        [[ -n "${x}" ]] && continue

        [[ -n "${older}" ]] && export "${excl_v}"="${!excl_v} ${d}"
        if [[ -f "${d}/${1}.bash" ]]; then
            if ! source "${d}/${1}.bash"; then
                type die &>/dev/null && eval die "\"Error loading module \${1}\""
                echo "Error loading module ${1}" 1>&2
                exit 123
            fi
            return
        else
            t="${t:+${t}, }${d}"
        fi
    done

    [[ -n "${older}" ]] && export "${excl_v}"="${save_excl}"

    type die &>/dev/null && eval die "\"Couldn't find module \${1} (looked in \${t})\""
    echo "Couldn't find module ${1} (looked in ${t})" 1>&2
    exit 123
}

ebuild_load_module pipe_functions
ebuild_load_module die_functions
ebuild_load_module output_functions
ebuild_load_module echo_functions
ebuild_load_module list_functions
ebuild_load_module source_functions

if [[ -z ${PALUDIS_LOAD_MODULES} ]]; then
    PALUDIS_LOAD_MODULES="
        conditional_functions kernel_functions sandbox sydbox portage_stubs
        multilib_functions install_functions build_functions"
    for m in eclass_functions exlib_functions ever_functions; do
        for d in ${EBUILD_MODULES_DIRS}; do
            if [[ -f "${d}/${1}.bash" ]]; then
                PALUDIS_LOAD_MODULES="${PALUDIS_LOAD_MODULES} ${m}"
                break
            fi
        done
    done
fi

for m in ${PALUDIS_LOAD_MODULES}; do
    ebuild_load_module ${m}
done

[[ -z ${PALUDIS_EBUILD_FUNCTIONS} ]] && PALUDIS_EBUILD_FUNCTIONS="
    builtin_infovars builtin_init builtin_initrm builtin_initmisc
    builtin_loadenv builtin_metadata builtin_killold builtin_killoldrm
    builtin_saveenv builtin_tidyup builtin_tidyuprm builtin_variable
    pkg_config pkg_info pkg_nofetch pkg_postinst pkg_postrm
    pkg_preinst pkg_prerm pkg_pretend pkg_setup pkg_bad_options
    src_compile src_configure src_install src_prepare src_test src_unpack
    src_fetch_extra"

# keep the upgrade from 0.36 to 0.38 working
[[ -z ${PALUDIS_EBUILD_PHASE_VAR} ]] && export PALUDIS_EBUILD_PHASE_VAR="EBUILD_PHASE"

check_paludis_pipe_command()
{
    [[ -n "${PALUDIS_SKIP_PIPE_COMMAND_CHECK}" ]] && return
    [[ -z "${PALUDIS_PIPE_COMMANDS_SUPPORTED}" ]] && return

    pcr=$(paludis_pipe_command PING DUNNOYET $$ )
    [[ "$pcr" == "PONG $$" ]] || die "paludis_pipe_command isn't working (got '$pcr')"
}

check_paludis_pipe_command

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

    local paludis_old_set=$-
    set -a

    if [[ -f ${1}/make.defaults ]] ; then
        source ${1}/make.defaults || die "Couldn't source ${1}/make.defaults"
    fi

    if [[ -f ${1}/bashrc ]] ; then
        source ${1}/bashrc || die "Couldn't source ${1}/bashrc"
    fi

    [[ ${paludis_old_set} == *a* ]] || set +a
}

ebuild_scrub_environment()
{
    local paludis_declared_functions
    paludis_declared_functions=$(declare -F | while read paludis_v ; do
        f=${paludis_v#declare -f }
        has ${f} ${PALUDIS_EBUILD_FUNCTIONS} || echo -n ${f} " "
    done )

    (
        ebuild_safe_source "${1}" PATH PALUDIS_SOURCE_MERGED_VARIABLES \
            PALUDIS_BRACKET_MERGED_VARIABLES LD_LIBRARY_PATH paludis_declared_functions || exit 1

        unset -v $(
            for v in ${!LD_*}; do
                [[ ${v} != LD_LIBRARY_PATH ]] && echo ${v}
            done )

        unset -f ${paludis_declared_functions}
        unset -v paludis_declared_functions

        if [[ "${2}" == "--pivot" ]] ; then
            unset -f ${PALUDIS_IGNORE_PIVOT_ENV_FUNCTIONS}
            unset -v ${PALUDIS_IGNORE_PIVOT_ENV_VARIABLES}
        fi

        unset -v ROOTPATH T TEMP HOME TMPDIR PORTDIR FILESDIR ECLASSDIR DISTDIR
        unset -v SKIP_FUNCTIONS FETCHEDDIR REPODIR EAPI FILES PKGMANAGER ROOT

        unset -v ${!GTKPALUDIS_CMDLINE_*} GTKPALUDIS_OPTIONS
        unset -v ${!ADJUTRIX_CMDLINE_*} ADJUTRIX_OPTIONS
        unset -v ${!QUALUDIS_CMDLINE_*} QUALUDIS_OPTIONS
        unset -v ${!RECONCILIO_CMDLINE_*} RECONCILIO_OPTIONS
        eval unset -v $(
            PALUDIS_CLIENT_UPPER=$(echo ${PALUDIS_CLIENT} | tr a-z A-Z)
            echo "\${!${PALUDIS_CLIENT_UPPER}_CMDLINE_*} ${PALUDIS_CLIENT_UPPER}_OPTIONS" )

        unset -v CATEGORY PN PV P PNV PVR PF PNVR
        unset -v ebuild EBUILD
        unset -v $(
            for v in ${PALUDIS_SOURCE_MERGED_VARIABLES} ${PALUDIS_BRACKET_MERGED_VARIABLES} ; do
                echo E_${v}
            done )
        unset -v ${!PALUDIS_*}

        unset -v $(
            for v in ${!SANDBOX_*}; do
                [[ ${v} != SANDBOX_ACTIVE ]] && echo ${v}
            done )
        export -n SANDBOX_ACTIVE

        unset -v $(
            for v in ${!BASH_*}; do
                [[ ${v#BASH_} != @(ARGC|ARGV|LINENO|SOURCE|VERSINFO|REMATCH) ]] && echo ${v}
            done )

        set >"${1}"
        print_exports >>"${1}"
    )
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
!!! should get an error that gives you a rough idea of where the
!!! problem lies.
"

        if [[ "${PALUDIS_LOAD_ENVIRONMENT%.bz2}" != "${PALUDIS_LOAD_ENVIRONMENT}" ]] ; then
            echo bunzip2 \< "${PALUDIS_LOAD_ENVIRONMENT}" \> ${PALUDIS_TMPDIR}/environment-${CATEGORY}-${!PALUDIS_NAME_VERSION_REVISION_VAR}-$$ 1>&2
            bunzip2 < "${PALUDIS_LOAD_ENVIRONMENT}" > ${PALUDIS_TMPDIR}/environment-${CATEGORY}-${!PALUDIS_NAME_VERSION_REVISION_VAR}-$$ \
                || die "Can't extract ${PALUDIS_LOAD_ENVIRONMENT}"
        else
            echo cp "${PALUDIS_LOAD_ENVIRONMENT}" "${PALUDIS_TMPDIR}/environment-${CATEGORY}-${!PALUDIS_NAME_VERSION_REVISION_VAR}-$$" 1>&2
            cp "${PALUDIS_LOAD_ENVIRONMENT}" "${PALUDIS_TMPDIR}/environment-${CATEGORY}-${!PALUDIS_NAME_VERSION_REVISION_VAR}-$$" \
                || die "Can't copy ${PALUDIS_LOAD_ENVIRONMENT}"
        fi

        echo ebuild_scrub_environment "${PALUDIS_TMPDIR}/environment-${CATEGORY}-${!PALUDIS_NAME_VERSION_REVISION_VAR}-$$" "$@" 1>&2
        ebuild_scrub_environment "${PALUDIS_TMPDIR}/environment-${CATEGORY}-${!PALUDIS_NAME_VERSION_REVISION_VAR}-$$" "$@" \
            || die "Can't load saved environment for cleaning"

        echo ebuild_safe_source "${PALUDIS_TMPDIR}/environment-${CATEGORY}-${!PALUDIS_NAME_VERSION_REVISION_VAR}-$$" 1>&2
        ebuild_safe_source "${PALUDIS_TMPDIR}/environment-${CATEGORY}-${!PALUDIS_NAME_VERSION_REVISION_VAR}-$$" \
            || die "Can't load saved environment"

        export PALUDIS_EXTRA_DIE_MESSAGE="${save_PALUDIS_EXTRA_DIE_MESSAGE}"

        echo rm "${PALUDIS_TMPDIR}/environment-${CATEGORY}-${!PALUDIS_NAME_VERSION_REVISION_VAR}-$$" 1>&2
        rm "${PALUDIS_TMPDIR}/environment-${CATEGORY}-${!PALUDIS_NAME_VERSION_REVISION_VAR}-$$"
    fi
}

ebuild_unset_all_except()
{
    local ${2}
    unset -v ${1}
}

ebuild_load_ebuild()
{
    local paludis_v paludis_e_v
    ebuild_unset_all_except "${EBUILD_METADATA_VARIABLES}" "${EBUILD_METADATA_VARIABLES_FROM_CPLUSPLUS}"
    unset -v ${PALUDIS_EBUILD_MUST_NOT_SET_VARIABLES}
    unset -v ${PALUDIS_SOURCE_MERGED_VARIABLES} ${PALUDIS_BRACKET_MERGED_VARIABLES}

    for paludis_v in ${PALUDIS_MUST_NOT_CHANGE_VARIABLES} ; do
        local paludis_saved_${paludis_v}
        eval paludis_saved_${paludis_v}='${!paludis_v}'
    done

    [[ -f ${1} ]] || die "Ebuild '${1}' is not a file"
    source ${1} || die "Error sourcing ebuild '${1}'"

    # we may or may not use this later
    PALUDIS_EBUILD_RDEPEND_WAS_SET=
    PALUDIS_EBUILD_DEPEND="${DEPEND}"
    [[ ${RDEPEND+set} != set ]] || PALUDIS_EBUILD_RDEPEND_WAS_SET=true

    for paludis_v in ${PALUDIS_SOURCE_MERGED_VARIABLES} ; do
        paludis_e_v=E_${paludis_v}
        eval ${paludis_v}='"${!paludis_v} ${!paludis_e_v}"'
    done

    for paludis_v in ${PALUDIS_BRACKET_MERGED_VARIABLES} ; do
        paludis_e_v=E_${paludis_v}
        if [[ -z "${!paludis_v}" ]] ; then
            eval ${paludis_v}='"${!paludis_e_v}"'
        elif has "${paludis_v}" ${PALUDIS_BRACKET_MERGED_VARIABLES_ANNOTATABLE} ; then
            eval ${paludis_v}='"( ${!paludis_v} ) [[ '\
                ${PALUDIS_BRACKET_MERGED_VARIABLES_ANNOTATION}' = [ '${1##*/}' ] ]] ${!paludis_e_v}"'
        else
            eval ${paludis_v}='"( ${!paludis_v} ) ${!paludis_e_v}"'
        fi
    done

    for paludis_v in ${PALUDIS_MUST_NOT_CHANGE_VARIABLES} ; do
        local paludis_s_v=paludis_saved_${paludis_v}
        if [[ -n ${!paludis_s_v} ]] && [[ ${!paludis_v} != ${!paludis_s_v} ]] ; then
            ebuild_notice "qa" \
                "Ebuild ${1} illegally tried to change ${paludis_v} from '${!paludis_s_v}' to '${!paludis_v}'"
            eval ${paludis_v}='${!paludis_s_v}'
        fi
    done

    for paludis_v in ${PALUDIS_MUST_NOT_CHANGE_AFTER_SOURCE_VARIABLES} ; do
        local paludis_s_v=PALUDIS_SAVE_GLOBAL_SCOPE_${paludis_v}
        export ${paludis_s_v}="$(declare -p ${paludis_v} 2>/dev/null)"
    done

    local paludis_p
    for paludis_p in ${PALUDIS_MUST_NOT_SET_VARS_STARTING_WITH} ; do
        for paludis_v in $(eval echo \${!${paludis_p}*}) ; do
            has ${paludis_v} ${PALUDIS_MUST_NOT_CHANGE_AFTER_SOURCE_VARIABLES} ||
                die "${paludis_v} starts with ${paludis_p} but isn't a known variable name"
        done
    done

    PALUDIS_DECLARED_FUNCTIONS=$(declare -F | while read paludis_v ; do
        echo -n ${paludis_v#declare -f } " "
    done )
}

ebuild_load_em_up_dan()
{
    export CONFIG_PROTECT=${PALUDIS_CONFIG_PROTECT}
    export CONFIG_PROTECT_MASK=${PALUDIS_CONFIG_PROTECT_MASK}
    local paludis_save_vars=$(eval echo ${PALUDIS_SAVE_VARIABLES} )
    local paludis_save_base_vars=$(eval echo ${PALUDIS_SAVE_BASE_VARIABLES} )
    local paludis_save_unmodifiable_vars=$(eval echo ${PALUDIS_SAVE_UNMODIFIABLE_VARIABLES} )
    local paludis_check_save_vars=${paludis_save_vars}
    local paludis_check_base_vars=${paludis_save_base_vars}
    local paludis_check_unmodifiable_vars=${paludis_save_unmodifiable_vars}

    local paludis_var
    for paludis_var in ${paludis_save_vars} ${paludis_save_base_vars} ${paludis_save_unmodifiable_vars} ; do
        local paludis_save_var_${paludis_var}
        eval paludis_save_var_${paludis_var}='${!paludis_var}'
    done

    if [[ -e ${ROOT}/etc/profile.env ]] && ! source "${ROOT}"/etc/profile.env; then
        echo "error sourcing ${ROOT}/etc/profile.env" >&2
        exit 123
    fi
    ebuild_sanitise_envvars

    if [[ -n ${PALUDIS_PROFILE_DIRS} ]] ; then
        for paludis_var in ${PALUDIS_PROFILE_DIRS} ; do
            ebuild_source_profile "$(canonicalise "${paludis_var}")"
        done
    elif [[ -n ${PALUDIS_PROFILE_DIR} ]] ; then
        ebuild_source_profile "$(canonicalise "${PALUDIS_PROFILE_DIR}")"
    fi

    unset ${paludis_save_vars} ${paludis_save_base_vars}

    local paludis_f
    for paludis_f in ${PALUDIS_BASHRC_FILES} ; do
        if [[ -f ${paludis_f} ]] ; then
            ebuild_notice "debug" "Loading bashrc file ${paludis_f}"
            local paludis_old_set=${-}
            set -a
            source ${paludis_f}
            [[ ${paludis_old_set} == *a* ]] || set +a
        else
            ebuild_notice "debug" "Skipping bashrc file ${paludis_f}"
        fi

        for paludis_var in ${paludis_check_save_vars} ; do
            if [[ -n ${!paludis_var} ]] ; then
                die "${paludis_f} attempted to set \$${paludis_var}, which must not be set in bashrc"
            fi
        done

        for paludis_var in ${paludis_check_save_unmodifiable_vars} ; do
            local paludis_s_var=paludis_save_var_${paludis_var}
            if [[ "${!paludis_s_var}" != "${!paludis_var}" ]] ; then
                die "${paludis_f} attempted to modify \$${var}, which must not be modified in bashrc"
            fi
        done
    done

    for paludis_var in ${paludis_save_vars} ; do
        local paludis_s_var=paludis_save_var_${paludis_var}
        eval ${paludis_var}='${!paludis_s_var}'
    done

    for paludis_var in ${paludis_save_base_vars} ; do
        local paludis_s_var=paludis_save_var_${paludis_var}
        eval ${paludis_var}='"${!paludis_s_var} $(echo ${!paludis_var})"'
    done

    if [[ -z ${PALUDIS_DO_NOTHING_SANDBOXY} ]] ; then
        if [[ -n ${CCACHE_DIR} ]]; then
            export SANDBOX_WRITE=${SANDBOX_WRITE}:${CCACHE_DIR}
            sydboxcheck 2>/dev/null && addwrite "${CCACHE_DIR}"
        fi
    fi

    [[ -z ${CBUILD} ]] && export CBUILD=${CHOST}
    export REAL_CHOST=${CHOST}

    ebuild_load_environment
    if [[ ${EBUILD} != - ]] ; then
        ebuild_load_ebuild "${EBUILD}"
    fi
}

perform_hook()
{
    export HOOK=${1}
    ebuild_notice "debug" "Starting hook '${HOOK}'"

    local old_sandbox_on="${SANDBOX_ON}"
    local old_sydbox_enabled
    sydboxcmd enabled 2>/dev/null && old_sydbox_enabled=true || old_sydbox_enabled=false
    if [[ -z "${PALUDIS_DO_NOTHING_SANDBOXY}" ]]; then
        export SANDBOX_ON="0"
        if sydboxcheck 2>/dev/null; then
            sydboxcmd off || ebuild_notice "warning" "sydboxcmd off returned failure"
        fi
    fi

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

    if [[ -z "${PALUDIS_DO_NOTHING_SANDBOXY}" ]]; then
        export SANDBOX_ON="${old_sandbox_on}"
        if sydboxcheck 2>/dev/null; then
            if $old_sydbox_enabled; then
                sydboxcmd on || ebuild_notice "warning" "sydboxcmd on returned failure"
            else
                sydboxcmd off || ebuild_notice "warning" "sydboxcmd off returned failure"
            fi
        fi
    fi
    true
}

paludis_phase_to_function_name() {
    local p
    for p in builtin src pkg; do
        if has ${p}_${1} ${PALUDIS_EBUILD_FUNCTIONS}; then
            echo ${p}_${1}
            return
        fi
    done
    die "Usage error: Unknown phase '${1}'"
}

ebuild_main()
{
    if ! [[ -e /proc/self ]] && [[ "$(uname -s)" == Linux ]] ; then
        ebuild_notice "warning" "/proc appears to be unmounted or unreadable."
        ebuild_notice "warning" "This will cause problems."
    fi

    # this is fatal in builtin_init, but warn early for good measure
    if [[ -z "${PALUDIS_TMPDIR}" ]] ; then
        ebuild_notice "warning" "PALUDIS_TMPDIR unset or empty."
    elif ! cd "${PALUDIS_TMPDIR}" ; then
        ebuild_notice "warning" "Could not change directory to ${PALUDIS_TMPDIR}."
    fi

    local action
    export EBUILD="${1}"
    shift

    ebuild_notice "debug" "Using ebuild '${EBUILD}', EAPI before source is '${EAPI}'"

    # If we're running under sydbox lock magic commands when execve() is called.
    if sydboxcheck 2>/dev/null; then
        sydboxcmd exec_lock || ebuild_notice "warning" "sydboxcmd exec_lock returned failure"
    fi

    if [[ ${#@} -ge 2 ]] ; then
        ebuild_section "Running ebuild phases $@ as $(id -un ):$(id -gn )..."
    elif [[ ${1} != variable ]] && [[ ${1} != metadata ]] && \
            [[ ${1} != pretend ]] && [[ ${1} != bad_options ]] ; then
        ebuild_section "Running ebuild phase $@ as $(id -un ):$(id -gn )..."
    fi

    for action in $@ ; do
        ebuild_load_module $(paludis_phase_to_function_name "${action}")
    done

    if [[ $1 == metadata ]] || [[ $1 == variable ]] || [[ $1 == pretend ]] || \
            [[ $1 == bad_options ]] ; then
        export ${PALUDIS_EBUILD_PHASE_VAR}="${1}"
        perform_hook ebuild_${action}_pre
        if [[ $1 == metadata ]]; then
            # Ban execve() calls if we're running under sydbox
            if sydboxcheck 2>/dev/null; then
                sydboxcmd sandbox/exec || ebuild_notice "warning" "sydboxcmd sandbox/exec returned failure"
            else
                for f in cut tr date ; do
                    eval "${f}() { ebuild_notice qa 'global scope ${f}' ; $(type -P ${f} ) \"\$@\" ; }"
                done
            fi
            for f in locked_pipe_command ; do
                eval "${f}() { $(type -P ${f} ) \"\$@\" ; }"
                if sydboxcheck 2>/dev/null; then
                    sydboxcmd addexec "$(type -P ${f})"
                fi
            done
            PATH="" ebuild_load_ebuild "${EBUILD}"
            # Unban execve() calls if we're running under sydbox
            if sydboxcheck 2>/dev/null; then
                sydboxcmd sandunbox/exec || ebuild_notice "warning" "sydboxcmd sandunbox/exec returned failure"
            fi
        else
            ebuild_load_em_up_dan
        fi
        if ! ${PALUDIS_F_FUNCTION_PREFIX:-ebuild_f}_${1} ; then
            perform_hook ebuild_${action}_fail
            die "${1} failed"
        fi
        perform_hook ebuild_${action}_post
    else
        for action in $@ ; do
            export ${PALUDIS_EBUILD_PHASE_VAR}="${action}"
            perform_hook ebuild_${action}_pre
            # Restrict network access to local if running under sydbox
            # We don't do sydboxcmd sand{un,}box/net here to allow the user set it in the configuration file.
            if [[ $action != unpack ]] && [[ $action != fetch_extra ]] ; then
                if sydboxcheck 2>/dev/null; then
                    sydboxcmd net/local || ebuild_notice "warning" "sydboxcmd net/local returned failure"
                    sydboxcmd net/restrict/connect || ebuild_notice "warning" "sydboxcmd net/restrict_connect return failure"
                fi
            fi
            if ! ${PALUDIS_F_FUNCTION_PREFIX:-ebuild_f}_${action} ; then
                if [[ $action != unpack ]] && [[ $action != fetch_extra ]] ; then
                    if sydboxcheck 2>/dev/null; then
                        sydboxcmd net/allow || ebuild_notice "warning" "sydboxcmd net/allow returned failure"
                        sydboxcmd net/unrestrict/connect || ebuild_notice "warning" "sydboxcmd net/unrestrict/connect returned failure"
                    fi
                fi
                perform_hook ebuild_${action}_fail
                die "${action} failed"
            fi
            if [[ $action != unpack ]] && [[ $action != fetch_extra ]] ; then
                if sydboxcheck 2>/dev/null; then
                    sydboxcmd net/allow || ebuild_notice "warning" "sydboxcmd net/allow returned failure"
                    sydboxcmd net/unrestrict/connect || ebuild_notice "warning" "sydboxcmd net/unrestrict/connect returned failure"
                fi
            fi
            perform_hook ebuild_${action}_post
        done
    fi

    if [[ ${#@} -ge 2 ]] ; then
        ebuild_section "Completed ebuild phases $@"
    elif [[ ${1} != variable ]] && [[ ${1} != metadata ]] && \
            [[ ${1} != pretend ]] && [[ ${1} != bad_options ]] ; then
        ebuild_section "Completed ebuild phase $@"
    fi
}

ebuild_main "$@"

