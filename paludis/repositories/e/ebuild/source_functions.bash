#!/usr/bin/env bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2007, 2008 David Leverton
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

ebuild_safe_source()
{
    # Set a list of variable names (and globs) that we want to filter out as positional arguments.
    # We'll use those in a fake signal handler later.
    set -- "${@}" '[^a-zA-Z_]*' '*[^a-zA-Z0-9_]*' \
        EUID PPID UID FUNCNAME GROUPS SHELLOPTS BASHOPTS BASHPID IFS PWD \
        'BASH_@(ARGC|ARGV|LINENO|SOURCE|VERSINFO|REMATCH)' \
        'BASH_COMPLETION?(_DIR)' 'bash+([0-9])?([a-z])' \
        EBUILD_KILL_PID PALUDIS_LOADSAVEENV_DIR PALUDIS_DO_NOTHING_SANDBOXY SANDBOX_ACTIVE \
        PALUDIS_IGNORE_PIVOT_ENV_FUNCTIONS PALUDIS_IGNORE_PIVOT_ENV_VARIABLES \
        PALUDIS_PIPE_COMMAND_READ_FD PALUDIS_PIPE_COMMAND_WRITE_FD

    # Clear the debug trap.
    trap DEBUG

    # Make sure that DEBUG traps are passed down to other processes as well.
    set -T

    # Enable some special handling of the DEBUG trap.
    # For instance, we skip executing commands if the debug trap exits with a non-zero status.
    shopt -s extdebug

    # Main filtering function, implemented as a DEBUG trap.
    # This will execute before every command. We can filter out commands by returning non-zero here.
    # A return value of 2 will halt the complete source statement. (This is not used here, but could
    # come in handy in case of errors?)
    # Understanding the filtering is a bit complicated:
    #   - First, we check for and drop preceding "declare" calls and their options by:
    #     o dropping everything past the first equal sign (this should leave us with a potential
    #       declare call, its options and the variable name)
    #     o dropping everything after the first space character and checking if that evaluates to a
    #       "declare" call
    #     o if it is, extract the variable name by dropping everything up to the last space
    #       character. This should drop the declare call and its options up until the variable name.
    #       This COULD create problems if the variable name contains one or multiple space
    #       characters, but we'll ignore that since, while such environment variables would
    #       generally be syntactically valid, they are not valid bash identifiers.
    #  - Then, check if the variable name matches any of our positional parameters. To do this, we
    #    concatenate them in a subshell(!) by settings the IFS variable to a pipe character and
    #    printing out all parameters through the ${*} shell variable.
    #    As an example, if the positional parameters are 'a', 'b', 'c' and 'd', this will generate
    #    this string: 'a|b|c|d'. Together with the extglob feature and a negation group, we can
    #    filter out the command by returning a non-zero exit value via the test utility.
    #  - Likewise, drop everything up until the first space character and check if that matches
    #    either a key-value pair (i.e., variable assignment) or the export or declare statements.
    #
    # Eventually, we might also want to include the "typeset" builtin, which is an alias for
    # "declare".
    trap "varname=\"\${BASH_COMMAND%%=*}\";
          [[ \${varname%%[[:space:]]*} == 'declare' ]] && {
              \$(: 'Skip over \"declare\" and options');
              varname=\"\${varname##*[[:space:]]}\";
          };
          [[ \${varname} == ?(*[[:space:]])!($(IFS='|'; shift; echo "${*}")) ||
              \${BASH_COMMAND%%[[:space:]]*} != @(*=*|export|declare) ]]" DEBUG

    # Sourcing a file with variables defined via declare or typeset in a function
    # will make the resulting variables local to the function.
    # We'll need them globally, though, so rewrite the statements to always include
    # a -g parameter.
    # We don't want to do this when creating the dump file, since portage doesn't
    # do it either and it actually makes sense to keep it pristine.
    #
    # To modify the original data, we have two options:
    #   - read in the data, modify it, write out to a tempfile and source the
    #     tempfile.
    #   - modify the data in place, then source it.
    #   - read in the data, modify and apply it directly.
    #
    # We'd like to avoid the first option, since that would require creating
    # temporary files.
    # The second option requires a GNU sed binary since in-place modification
    # is a bit complicated.
    # The third option requires switching from the source builtin to the eval
    # builtin, since only files can be source'd.
    # Additionally, even if source supported reading and applying from stdin,
    # we couldn't use pipes or subshells since that would create a new process
    # space. Applying any new variables in there wouldn't affect the parent,
    # but we actually intend to do exactly that.
    # Now, the "issue" with eval is that we won't be able to use the special
    # "call return while executing the source builtin if the DEBUG trap's return
    # value was 2" property, so we'll settle on option two.
    sed -i -e 's/^declare -/declare -g -/' "${1}" && source "${1}"

    eval "trap DEBUG; shopt -u extdebug; set +T; return ${?}"
}
ebuild_need_extglob ebuild_safe_source

ebuild_verify_not_changed_from_global_scope()
{
    local v vv_orig
    for v in "$@" ; do
        vv_orig=PALUDIS_SAVE_GLOBAL_SCOPE_$v
        vv_orig=${!vv_orig}
        [[ "${vv_orig}" == "$(declare -p "${v}" 2>/dev/null)" ]] || die "Variable $v must be set to an invariant value in global scope"
    done
}

