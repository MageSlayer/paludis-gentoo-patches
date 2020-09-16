#!/usr/bin/env bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011, 2013 Ciaran McCreesh
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

ebuild_need_extglob()
{
    eval "_ebuild_need_extglob_$(declare -f ${1})"
    eval "
        ${1}()
        {
            eval \"
                shopt -s extglob
                _ebuild_need_extglob_${1} \\\"\\\${@}\\\"
                eval \\\"\$(shopt -p extglob); return \\\${?}\\\"
            \"
        }"
}

ebuild_cleanup_slashes()
{
    export "${1}=${!1//+(\/)//}"
    export "${1}=${!1/%*(\/)}/"
}
ebuild_need_extglob ebuild_cleanup_slashes

ebuild_sanitise_envvars()
{
    local p

    # Force a few more things into PATH, since some users have crazy setups.
    # See ticket:374.
    export PATH="/usr/bin:/usr/sbin:/bin:/sbin${PATH:+:${PATH}}"

    [[ -n "${BANNEDDIR}" ]] && export PATH="${BANNEDDIR}:${PATH}"

    export PATH="${PALUDIS_EBUILD_DIR}/utils:${PATH}"
    # Automake likes to scatter our utilities over two directories.
    [[ -n "${PALUDIS_EBUILD_DIR_FALLBACK}" ]] && export PATH="${PALUDIS_EBUILD_DIR_FALLBACK}/utils:${PATH}"
    for p in ${PALUDIS_UTILITY_PATH_SUFFIXES} ; do
        export PATH="${PALUDIS_EBUILD_DIR}/utils/${p}:${PATH}"
        [[ -n "${PALUDIS_EBUILD_DIR_FALLBACK}" ]] && export PATH="${PALUDIS_EBUILD_DIR_FALLBACK}/utils/${p}:${PATH}"
    done

    unset GZIP BZIP BZIP2 CDPATH GREP_OPTIONS GREP_COLOR GLOBIGNORE
    unset LANG ${!LC_*}
    export LC_ALL=C

    # pagers won't run with redirected output, but some naughty packages like
    # to use them.
    export PAGER=cat
}
ebuild_sanitise_envvars

# The list below should include all variables from all EAPIs, along with any
# fancy fake variables
EBUILD_METADATA_VARIABLES="DEPEND RDEPEND PDEPEND IUSE IUSE_EFFECTIVE SRC_URI DOWNLOADS RESTRICT \
    LICENSE LICENCES KEYWORDS INHERITED PROVIDE HOMEPAGE DESCRIPTION DEPENDENCIES \
    E_IUSE E_DEPEND E_RDEPEND E_PDEPEND PLATFORMS DEFINED_PHASES EBUILD_PHASE_FUNC \
    MYOPTIONS E_MYOPTIONS E_DEPENDENCIES BINARY_KEYWORDS BINARY_URI \
    GENERATED_USING GENERATED_TIME GENERATED_FROM_REPOSITORY BINARY_PLATFORMS REMOTE_IDS \
    SUMMARY BUGS_TO UPSTREAM_DOCUMENTATION UPSTREAM_CHANGELOG \
    UPSTREAM_RELEASE_NOTES PROPERTIES PALUDIS_DECLARED_FUNCTIONS SLOT EAPI OPTIONS USE \
    PALUDIS_EBUILD_RDEPEND_WAS_SET PALUDIS_EBUILD_DEPEND REQUIRED_USE SCM_REVISION"
EBUILD_METADATA_VARIABLES_FROM_CPLUSPLUS="SLOT EAPI OPTIONS USE IUSE_EFFECTIVE"

export -n BASH_COMPAT=${PALUDIS_BASH_COMPAT}
shopt -s expand_aliases
[[ -z ${PALUDIS_SHELL_OPTIONS} && unset == ${PALUDIS_SHELL_OPTIONS-unset} ]] &&
    shopt -s extglob
for p in ${PALUDIS_SHELL_OPTIONS} ; do
    shopt -p|grep -q ${p} && shopt -s ${p}
done
for p in ${PALUDIS_SHELL_OPTIONS_DISABLED} ; do
    shopt -p|grep -q ${p} && shopt -u ${p}
done

ebuild_cleanup_slashes ROOT

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
                exit 124
            fi
            return
        else
            t="${t:+${t}, }${d}"
        fi
    done

    [[ -n "${older}" ]] && export "${excl_v}"="${save_excl}"

    type die &>/dev/null && eval die "\"Couldn't find module \${1} (looked in \${t})\""
    echo "Couldn't find module ${1} (looked in ${t})" 1>&2
    exit 125
}
ebuild_need_extglob ebuild_load_module

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
    builtin_bad_required_use
    pkg_config pkg_info pkg_nofetch pkg_postinst pkg_postrm
    pkg_preinst pkg_prerm pkg_pretend pkg_setup pkg_bad_options
    src_compile src_configure src_install src_prepare src_test src_unpack
    src_fetch_extra"

# keep the upgrade from 0.36 to 0.38 working
[[ -z ${PALUDIS_EBUILD_PHASE_VAR} ]] && export PALUDIS_EBUILD_PHASE_VAR="EBUILD_PHASE"

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

    if esandbox check 2>/dev/null; then
        esandbox allow \
            /dev/{stdout,stderr,zero,null,full,console,random,ptmx} \
            /dev/{fd,tts,pts,shm,tty,pty} \
            /proc/self/{fd,attr,task} \
            /selinux/context \
            /tmp /var/tmp /var/cache \
            "${PALUDIS_TMPDIR%/}"
        if [[ -n "${CCACHE_DIR}" ]]; then
            esandbox allow "${CCACHE_DIR%/}"
        fi

        esandbox allow_net \
            LOOPBACK@0 \
            LOOPBACK@1024-65535 \
            LOOPBACK6@0 \
            LOOPBACK6@1024-65535

        esandbox allow_net --connect \
            unix:/var/run/nscd/socket \
            unix:/run/nscd/socket
    fi
fi

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
    if [[ -z ${PALUDIS_PROFILES_DIRS} ]]; then
        if [[ -f ${1}/parent ]] ; then
            while read line ; do
                grep --silent '^[[:space:]]*#' <<<"${line}" && continue
                grep --silent '[^[:space:]]' <<<"${line}" || continue
                ebuild_source_profile $(canonicalise ${1}/${line} )
            done <${1}/parent
        fi
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
        unset -v ebuild EBUILD MERGE_TYPE
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
                case "${v#BASH_}" in
                    (ARGC|ARGV|LINENO|SOURCE|VERSINFO|REMATCH) : ;;
                    (*) echo ${v} ;;
                esac
            done
        )

        declare -p >"${1}"
        declare -pf >>"${1}"
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

    local paludis_shopts=$(shopt -p)
    [[ -n ${PALUDIS_SHELL_OPTIONS_GLOBAL} ]] && shopt -s ${PALUDIS_SHELL_OPTIONS_GLOBAL}

    if [[ ! -f ${1} ]]; then
        [[ -r ${1} ]] || die "Ebuild '${1}' cannot be read"
        die "Ebuild '${1}' is not a file"
    fi
    source ${1} || die "Error sourcing ebuild '${1}'"

    eval "${paludis_shopts}"

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
        exit 126
    fi
    ebuild_sanitise_envvars

    if [[ -n ${PALUDIS_PROFILES_DIRS:-${PALUDIS_PROFILE_DIRS}} ]] ; then
        for paludis_var in ${PALUDIS_PROFILES_DIRS:-${PALUDIS_PROFILE_DIRS}} ; do
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
            esandbox check 2>/dev/null && esandbox allow "${CCACHE_DIR}"
        fi
        if [[ -n ${TMPDIR} ]] && esandbox check 2>/dev/null; then
            local tmpdirc="$(canonicalise "${TMPDIR}")"
            esandbox allow_net unix:"${tmpdirc}" unix-abstract:"${tmpdirc}"
            unset tmpdirc
        fi
    fi

    umask 022
    export DESTTREE="/usr"
    export INSDESTTREE=""
    export EXEDESTTREE=""
    export DOCDESTTREE=""
    export INSOPTIONS="-m0644"
    export EXEOPTIONS="-m0755"
    export LIBOPTIONS="-m0644"
    export DIROPTIONS="-m0755"
    export MOPREFIX="${PN}"

    # We need to set variables like CHOST, CC, CFLAGS, etc. properly after
    # profiles and user bashrc have been source but before the exheres get
    # sourced.
    if [[ ${FILESYSTEM_LAYOUT} == cross ]] ; then
        # This is a really icky place to push this, but it needs to be before
        # any phase is called and after profiles were loaded.
        DESTTREE=/usr/$(exhost --target)

        local tool= cross_tool_prefix=$(exhost --tool-prefix)
        local flags= host_flags= host_prepend_flags= cross_flags_prefix=$(exhost --target)

        # TODO(zlin) figure out if this should just bee hardcoded here rather
        # than placed into profiles (as is currently)
        #   CROSS_COMPILE_TOOLS="AR:ar AS:as CC:gcc CPP:cpp CXX:g++ FORTRAN:gfortran LD:ld NM:nm RANLIB:ranlib"
        for tool in ${CROSS_COMPILE_TOOLS} ; do
            export ${tool%:*}=${cross_tool_prefix}${tool#*:}
        done

        # TODO(zlin) figure out if this should just be hardcoded here rather
        # than placed into profiles (as is currently)
        #   CROSS_COMPILE_FLAGS="CFLAGS CPPFLAGS:CFLAGS CXXFLAGS LDFLAGS"
        for flags in ${CROSS_COMPILE_FLAGS} ; do
            if [[ ${flags} == *:* ]] ; then
                host_prepend_flags=${cross_flags_prefix//-/_}_${flags#*:}
            else
                host_prepend_flags=
            fi

            flags=${flags%:*}

            host_flags=${cross_flags_prefix//-/_}_${flags}
            [[ -n ${!host_flags} ]] && export ${flags}="${!host_flags}"
            if [[ -n ${host_prepend_flags} && -n ${!host_prepend_flags} ]]
            then
                export ${flags}="${!host_prepend_flags}${!flags:+ }${!flags}"
            fi
        done
    else
        [[ -z ${CBUILD} ]] && export CBUILD=${CHOST}
        export REAL_CHOST=${CHOST}
    fi

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
    local old_box_enabled
    esandbox enabled 2>/dev/null && old_box_enabled=true || old_box_enabled=false
    if [[ -z "${PALUDIS_DO_NOTHING_SANDBOXY}" ]]; then
        export SANDBOX_ON="0"
        if esandbox check 2>/dev/null; then
            esandbox disable || ebuild_notice "warning" "esandbox disable returned failure"
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
        if esandbox check 2>/dev/null; then
            if $old_box_enabled; then
                esandbox enable || ebuild_notice "warning" "esandbox enable returned failure"
            else
                esandbox disable || ebuild_notice "warning" "esandbox disable returned failure"
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

    # If we're running under sandbox lock magic commands when execve() is called.
    if esandbox check 2>/dev/null; then
        esandbox exec_lock || ebuild_notice "warning" "esandbox exec_lock returned failure"
    fi

    if [[ ${#@} -ge 2 ]] ; then
        ebuild_section "Running ebuild phases $@ as $(id -un ):$(id -gn )..."
    else
        ebuild_section "Running ebuild phase $@ as $(id -un ):$(id -gn )..."
    fi

    for action in $@ ; do
        ebuild_load_module $(paludis_phase_to_function_name "${action}")
    done

    for action in $@ ; do
        export ${PALUDIS_EBUILD_PHASE_VAR}="${action}"
        [[ -n ${PALUDIS_EBUILD_PHASE_FUNC_VAR} ]] && export ${PALUDIS_EBUILD_PHASE_FUNC_VAR}="$(paludis_phase_to_function_name "${action}")"

        perform_hook ebuild_${action}_pre

        if [[ ${action} == metadata ]]; then
            # Ban execve() calls if we're running under sandbox
            if esandbox check 2>/dev/null; then
                esandbox enable_exec || ebuild_notice "warning" "esandbox enable_exec returned failure"
            else
                for f in cut tr date ; do
                    eval "${f}() { ebuild_notice qa 'global scope ${f}' ; $(type -P ${f} ) \"\$@\" ; }"
                done
            fi
            for f in locked_pipe_command ; do
                eval "${f}() { $(type -P ${f} ) \"\$@\" ; }"
                if esandbox check 2>/dev/null; then
                    esandbox allow_exec "$(type -P ${f})"
                fi
            done
            PATH="" ebuild_load_ebuild "${EBUILD}"
            # Unban execve() calls if we're running under sandbox
            if esandbox check 2>/dev/null; then
                esandbox disable_exec || ebuild_notice "warning" "esandbox disable_exec returned failure"
            fi
        fi

        if [[ ${#@} -eq 1 ]] && [[ ${action} == variable || ${action} == pretend || ${action} == bad_options ]]; then
            ebuild_load_em_up_dan
        fi

        # Restrict network access if running under sandbox
        if [[ $action != unpack ]] && [[ $action != fetch_extra ]] ; then
            if esandbox check 2>/dev/null; then
                esandbox enable_net || ebuild_notice "warning" "esandbox enable_net returned failure"
            fi
        fi

        ${PALUDIS_F_FUNCTION_PREFIX:-ebuild_f}_${action}
        local paludis_ebuild_phase_status="${?}"

        if [[ $action != unpack ]] && [[ $action != fetch_extra ]] ; then
            if esandbox check 2>/dev/null; then
                esandbox disable_net || ebuild_notice "warning" "esandbox disable_net returned failure"
            fi
        fi

        if [[ ${paludis_ebuild_phase_status} -ne 0 ]]; then
            perform_hook ebuild_${action}_fail
            die "${action} failed"
        fi
        perform_hook ebuild_${action}_post
    done

    if [[ ${#@} -ge 2 ]] ; then
        ebuild_section "Completed ebuild phases $@"
    else
        ebuild_section "Completed ebuild phase $@"
    fi
}

ebuild_main "$@"

