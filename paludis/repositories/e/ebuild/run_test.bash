#!/usr/bin/env bash
# vim: set sw=4 sts=4 et :

shopt -s expand_aliases
shopt -s extglob
set +o posix

export TEST_STATUS=
export PALUDIS_IN_TEST_FRAMEWORK="yes"
unset PALUDIS_UTILITY_PATH_SUFFIXES

if [[ -z "${PALUDIS_EBUILD_MODULE_SUFFIXES}" ]] ; then
    echo "Eek! PALUDIS_EBUILD_MODULE_SUFFIXES unset or empty"
    exit 123
fi

test_return_code()
{
    local r="$?"
    if [[ "0" == "${r}" ]] ; then
        echo -n "."
    else
        echo -n "!{retcode: ${r}}"
        export local_test_status="fail"
        export TEST_STATUS="fail"
    fi
}

test_equality()
{
    if [[ "${1}" == "${2}" ]] ; then
        echo -n "."
    else
        echo -n "!{'${1}' not equal to '${2}'}"
        export local_test_status="fail"
        export TEST_STATUS="fail"
    fi
}

export PALUDIS_PIPE_COMMAND_WRITE_FD=
export PALUDIS_PIPE_COMMAND_READ_FD=
export PALUDIS_SKIP_PIPE_COMMAND_CHECK=yes
export PALUDIS_LOAD_MODULES=

echo "Test program ${1}:"
source "${PALUDIS_EBUILD_DIR}/ebuild.bash" || exit 200
source "${1}" || exit 200

paludis_pipe_command()
{
    echo "No paludis_pipe_command here"
    exit 127
}

for testname in $(set | grep '_TEST *() *$' ) ; do
    [[ ${testname/()} != ${testname} ]] && continue
    echo -n "* ${testname%_TEST}: "
    export local_test_status=""
    ${testname}
    [[ -z "$local_test_status" ]] && echo " OK" || echo " FAIL"
done

[[ -z "$TEST_STATUS" ]]

