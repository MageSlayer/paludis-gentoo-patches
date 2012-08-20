#!/usr/bin/env bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2009, 2010, 2011, 2012 Ali Polatel <alip@exherbo.org>
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

sydbox_internal_api()
{
    if [[ -e /dev/sydbox/1 ]]; then
        echo -n 1
    else
        echo -n 0
    fi
}

sydbox_internal_cmd()
{
    if [[ -e /dev/sydbox ]]; then
        if [[ -n "${2}" ]]; then
            [[ -e /dev/sydbox/${1}/"${2}" ]]
        else
            [[ -e /dev/sydbox/${1} ]]
        fi
    fi
}

sydbox_internal_path()
{
    local cmd="${1}"
    shift
    for path in "${@}"; do
        [[ "${path:0:1}" == '/' ]] || die "${FUNCNAME} expects absolute path, got: ${path}"
        sydbox_internal_cmd "${cmd}" "${path}"
    done
}

sydbox_internal_path_1()
{
    local cmd="${1}"
    local op="${2}"

    case "${op}" in
    '+'|'-')
        ;;
    *)
        die "${FUNCNAME}: invalid operation character '${op}'"
        ;;
    esac

    shift 2

    for path in "${@}"; do
        [[ "${path:0:1}" == '/' ]] || die "${FUNCNAME} expects absolute path, got: ${path}"
        [[ -e /dev/sydbox/"${cmd}${op}${path}" ]]
    done
}

sydbox_internal_net()
{
    local cmd="${1}"

    shift
    while [[ ${#} > 0 ]] ; do
        case "${1}" in
        inet6:*)
            sydbox_internal_cmd "${cmd}/inet6://${1##inet6:}"
            ;;
        inet:*)
            sydbox_internal_cmd "${cmd}/inet://${1##inet:}"
            ;;
        unix-abstract:*)
            sydbox_internal_cmd "${cmd}/unix-abstract://${1##unix-abstract:}"
            ;;
        unix:*)
            sydbox_internal_cmd "${cmd}/unix://${1##unix:}"
            ;;
        *)
            # Expect network alias.
            # Sydbox does input validation so we don't do any here.
            sydbox_internal_cmd "${cmd}/${1}"
            ;;
        esac
        shift
    done
}

sydbox_internal_net_1()
{
    local cmd="${1}"
    local op="${2}"

    case "${op}" in
    '+'|'-')
        ;;
    *)
        die "${FUNCNAME}: invalid operation character '${op}'"
        ;;
    esac

    shift 2

    for addr in "${@}"; do
        [[ -e /dev/sydbox/"${cmd}${op}${addr}" ]]
    done
}

esandbox_1()
{
    local cmd="${1}"

    shift
    case "${cmd}" in
    api)
        echo -n 1
        ;;
    check)
        [[ -e /dev/sydbox ]]
        ;;
    lock)
        [[ -e "/dev/sydbox/core/trace/magic_lock:on" ]]
        ;;
    exec_lock)
        [[ -e "/dev/sydbox/core/trace/magic_lock:exec" ]]
        ;;
    wait_all)
        [[ -e "/dev/sydbox/core/trace/exit_wait_all:true" ]]
        ;;
    wait_eldest)
        [[ -e "/dev/sydbox/core/trace/exit_wait_all:false" ]]
        ;;
    enabled|enabled_path)
        [[ -e "/dev/sydbox/core/sandbox/write?" ]]
        ;;
    enable|enable_path)
        [[ -e "/dev/sydbox/core/sandbox/write:deny" ]]
        ;;
    disable|disable_path)
        [[ -e "/dev/sydbox/core/sandbox/write:off" ]]
        ;;
    enabled_exec)
        [[ -e "/dev/sydbox/core/sandbox/exec?" ]]
        ;;
    enable_exec)
        [[ -e "/dev/sydbox/core/sandbox/exec:deny" ]]
        ;;
    disable_exec)
        [[ -e "/dev/sydbox/core/sandbox/exec:off" ]]
        ;;
    enabled_net)
        [[ -e "/dev/sydbox/core/sandbox/network?" ]]
        ;;
    enable_net)
        [[ -e "/dev/sydbox/core/sandbox/network:deny" ]]
        ;;
    disable_net)
        [[ -e "/dev/sydbox/core/sandbox/network:off" ]]
        ;;
    allow|allow_path)
        [[ ${#} < 1 ]] && die "${FUNCNAME} ${cmd} takes at least one extra argument"
        sydbox_internal_path_1 "whitelist/write" '+' "${@}"
        ;;
    disallow|disallow_path)
        [[ ${#} < 1 ]] && die "${FUNCNAME} ${cmd} takes at least one extra argument"
        sydbox_internal_path_1 "whitelist/write" '-' "${@}"
        ;;
    allow_exec)
        [[ ${#} < 1 ]] && die "${FUNCNAME} ${cmd} takes at least one extra argument"
        sydbox_internal_path_1 "whitelist/exec" '+' "${@}"
        ;;
    disallow_exec)
        [[ ${#} < 1 ]] && die "${FUNCNAME} ${cmd} takes at least one extra argument"
        sydbox_internal_path_1 "whitelist/exec" '-' "${@}"
        ;;
    allow_net)
        local c="whitelist/network/bind"
        [[ "${1}" == "--connect" ]] && c="whitelist/network/connect" && shift
        [[ ${#} < 1 ]] && die "${FUNCNAME} ${cmd} takes at least one extra argument"
        sydbox_internal_net_1 "${c}" '+' "${@}"
        ;;
    disallow_net)
        local c="whitelist/network/bind"
        [[ "${1}" == "--connect" ]] && c="whitelist/network/connect" && shift
        [[ ${#} < 1 ]] && die "${FUNCNAME} ${cmd} takes at least one extra argument"
        sydbox_internal_net_1 "${c}" '-' "${@}"
        ;;
    addfilter|addfilter_path)
        [[ ${#} < 1 ]] && die "${FUNCNAME} ${cmd} takes at least one extra argument"
        sydbox_internal_path_1 "filter/write" '+' "${@}"
        ;;
    rmfilter|rmfilter_path)
        [[ ${#} < 1 ]] && die "${FUNCNAME} ${cmd} takes at least one extra argument"
        sydbox_internal_path_1 "filter/write" '-' "${@}"
        ;;
    addfilter_exec)
        [[ ${#} < 1 ]] && die "${FUNCNAME} ${cmd} takes at least one extra argument"
        sydbox_internal_path_1 "filter/exec" '+' "${@}"
        ;;
    rmfilter_exec)
        [[ ${#} < 1 ]] && die "${FUNCNAME} ${cmd} takes at least one extra argument"
        sydbox_internal_path_1 "filter/exec" '-' "${@}"
        ;;
    addfilter_net)
        [[ ${#} < 1 ]] && die "${FUNCNAME} ${cmd} takes at least one extra argument"
        sydbox_internal_net_1 "filter/network" '+' "${@}"
        ;;
    rmfilter_net)
        [[ ${#} < 1 ]] && die "${FUNCNAME} ${cmd} takes at least one extra argument"
        sydbox_internal_net_1 "filter/network" '-' "${@}"
        ;;
    kill)
        [[ ${#} < 1 ]] && die "${FUNCNAME} ${cmd} takes at least one extra argument"
        sydbox_internal_path_1 "exec/kill_if_match" "+" "${@}"
        ;;
    resume)
        [[ ${#} < 1 ]] && die "${FUNCNAME} ${cmd} takes at least one extra argument"
        sydbox_internal_path_1 "exec/resume_if_match" "+" "${@}"
        ;;
    hack_toolong|nohack_toolong)
        ebuild_notice "warning" "${FUNCNAME} ${cmd} is not implemented for sydbox-1"
        false;;
    *)
        die "${FUNCNAME} subcommand ${cmd} unrecognised"
        ;;
    esac
}

esandbox_0() {
    local cmd="${1}"

    shift
    case "${cmd}" in
    api)
        echo -n 0
        ;;
    check)
        [[ -e /dev/sydbox ]]
        ;;
    lock)
        sydbox_internal_cmd lock
        ;;
    exec_lock)
        sydbox_internal_cmd exec_lock
        ;;
    wait_all)
        sydbox_internal_cmd wait/all
        ;;
    wait_eldest)
        sydbox_internal_cmd wait/eldest
        ;;
    hack_toolong)
        sydbox_internal_cmd wrap/lstat
        ;;
    nohack_toolong)
        sydbox_internal_cmd nowrap/lstat
        ;;
    enabled|enabled_path)
        sydbox_internal_cmd enabled
        ;;
    enable|enable_path)
        sydbox_internal_cmd on
        ;;
    disable|disable_path)
        sydbox_internal_cmd off
        ;;
    enabled_exec)
        ebuild_notice "warning" "${FUNCNAME} ${cmd} is not implemented for sydbox"
        false;;
    enable_exec)
        sydbox_internal_cmd sandbox/exec
        ;;
    disable_exec)
        sydbox_internal_cmd sandunbox/exec
        ;;
    enabled_net)
        ebuild_notice "warning" "${FUNCNAME} ${cmd} is not implemented for sydbox"
        false;;
    enable_net)
        sydbox_internal_cmd sandbox/net
        ;;
    disable_net)
        sydbox_internal_cmd sandunbox/net
        ;;
    allow|allow_path)
        [[ ${#} < 1 ]] && die "${FUNCNAME} ${cmd} takes at least one extra argument"
        sydbox_internal_path "write" "${@}"
        ;;
    disallow|disallow_path)
        [[ ${#} < 1 ]] && die "${FUNCNAME} ${cmd} takes at least one extra argument"
        sydbox_internal_path "unwrite" "${@}"
        ;;
    allow_exec)
        [[ ${#} < 1 ]] && die "${FUNCNAME} ${cmd} takes at least one extra argument"
        sydbox_internal_path "addexec" "${@}"
        ;;
    disallow_exec)
        [[ ${#} < 1 ]] && die "${FUNCNAME} ${cmd} takes at least one extra argument"
        sydbox_internal_path "rmexec" "${@}"
        ;;
    allow_net)
        local c="net/whitelist/bind"
        [[ "${1}" == "--connect" ]] && c="net/whitelist/connect" && shift
        [[ ${#} < 1 ]] && die "${FUNCNAME} ${cmd} takes at least one extra argument"
        sydbox_internal_net "${c}" "${@}"
        ;;
    disallow_net)
        local c="net/unwhitelist/bind"
        [[ "${1}" == "--connect" ]] && c="net/unwhitelist/connect" && shift
        [[ ${#} < 1 ]] && die "${FUNCNAME} ${cmd} takes at least one extra argument"
        sydbox_internal_net "${c}" "${@}"
        ;;
    addfilter|addfilter_path)
        [[ ${#} < 1 ]] && die "${FUNCNAME} ${cmd} takes at least one extra argument"
        sydbox_internal_path "addfilter" "${@}"
        ;;
    rmfilter|rmfilter_path)
        [[ ${#} < 1 ]] && die "${FUNCNAME} ${cmd} takes at least one extra argument"
        sydbox_internal_path "rmfilter" "${@}"
        ;;
    addfilter_exec)
        [[ ${#} < 1 ]] && die "${FUNCNAME} ${cmd} takes at least one extra argument"
        sydbox_internal_path "addfilter_exec" "${@}"
        ;;
    rmfilter_exec)
        [[ ${#} < 1 ]] && die "${FUNCNAME} ${cmd} takes at least one extra argument"
        sydbox_internal_path "rmfilter_exec" "${@}"
        ;;
    addfilter_net)
        [[ ${#} < 1 ]] && die "${FUNCNAME} ${cmd} takes at least one extra argument"
        sydbox_internal_net "addfilter_net" "${@}"
        ;;
    rmfilter_net)
        [[ ${#} < 1 ]] && die "${FUNCNAME} ${cmd} takes at least one extra argument"
        sydbox_internal_net "rmfilter_net" "${@}"
        ;;
    kill|resume)
        ebuild_notice "warning" "${FUNCNAME} ${cmd} is not implemented for sydbox-0"
        ;;
    *)
        die "${FUNCNAME} subcommand ${cmd} unrecognised"
        ;;
    esac
}

esandbox() {
    local api

    api="$(sydbox_internal_api)"
    case "${api}" in
    1)
        esandbox_1 "${@}";;
    0)
        esandbox_0 "${@}";;
    *)
        die "${FUNCNAME}: unrecognised sydbox API '${api}'"
        ;;
    esac
}

sydboxcheck()
{
    ebuild_notice "warning" "${FUNCNAME} is deprecated, use \"esandbox check\" instead"
    esandbox check
}

sydboxcmd()
{
    ebuild_notice "warning" "${FUNCNAME} is deprecated, use \"esandbox <command>\" instead"
    sydbox_internal_cmd "${@}"
}

addread()
{
    die "${FUNCNAME} not implemented for sydbox yet"
}

addwrite()
{
    ebuild_notice "warning" "${FUNCNAME} is deprecated, use \"esandbox allow\" instead"
    esandbox allow "${1}"
}

adddeny()
{
    die "${FUNCNAME} not implemented for sydbox yet"
}

addpredict()
{
    die "${FUNCNAME} is dead, use \"esandbox addfilter\" instead"
}

rmwrite()
{
    ebuild_notice "warning" "${FUNCNAME} is deprecated, use \"esandbox disallow\" instead"
    esandbox disallow "${1}"
}

rmpredict()
{
    die "${FUNCNAME} is dead, use \"esandbox rmfilter\" instead"
}

addfilter()
{
    ebuild_notice "warning" "${FUNCNAME} is deprecated, use \"esandbox addfilter\" instead"
    esandbox addfilter "${1}"
}

rmfilter()
{
    ebuild_notice "warning" "${FUNCNAME} is deprecated, use \"esandbox rmfilter\" instead"
    esandbox rmfilter "${1}"
}

