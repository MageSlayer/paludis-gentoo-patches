#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

testname=${2:-${1}}
interp=
[[ ${testname%.py} != ${testname} ]] && interp="${PYTHON}"
[[ ${testname%.rb} != ${testname} ]] && interp="${RUBY} -I ./.libs/"

testname=${testname%.rb}
testname=${testname%.py}
testname=${testname%.bash}

[[ -z "PALUDIS_TESTS_REAL_STDOUT_FD" ]] && export PALUDIS_TESTS_REAL_STDOUT_FD=1

if test -f "$TEST_SCRIPT_DIR""${testname}"_"cleanup.sh" ; then
    echo ">>> cleanup for test ${testname}" 1>&$PALUDIS_TESTS_REAL_STDOUT_FD
    if ! "$TEST_SCRIPT_DIR""${testname}"_"cleanup.sh" ; then
        echo ">>> exiting with error for test ${testname}" 1>&$PALUDIS_TESTS_REAL_STDOUT_FD
        exit 255
    fi
else
    echo ">>> No $TEST_SCRIPT_DIR${testname}_cleanup.sh to run" 1>&$PALUDIS_TESTS_REAL_STDOUT_FD
fi

if test -f "$TEST_SCRIPT_DIR""${testname}"_"setup.sh" ; then
    echo ">>> setup for test ${testname}" 1>&$PALUDIS_TESTS_REAL_STDOUT_FD
    if ! "$TEST_SCRIPT_DIR""${testname}"_"setup.sh" ; then
        echo ">>> exiting with error for test ${testname}" 1>&$PALUDIS_TESTS_REAL_STDOUT_FD
        exit 255
    fi
else
    echo ">>> No $TEST_SCRIPT_DIR${testname}_setup.sh to run" 1>&$PALUDIS_TESTS_REAL_STDOUT_FD
fi

echo ">>> test ${testname}" 1>&$PALUDIS_TESTS_REAL_STDOUT_FD 1>&$PALUDIS_TESTS_REAL_STDOUT_FD
$interp ${@}
code=$?

if [[ 0 != ${code} ]] ; then
    echo ">>> test ${testname} returned ${code}" 1>&$PALUDIS_TESTS_REAL_STDOUT_FD
    if [[ -z "${PALUDIS_TESTS_RERUN_VERBOSELY}" ]] && [[ "${testname#./}" != "test_fail_TEST" ]] ; then
        out=`pwd`/${testname#./}.epicfail
        echo ">>> rerunning test ${testname} verbosely redirected to ${out}" 1>&$PALUDIS_TESTS_REAL_STDOUT_FD
        env PALUDIS_TESTS_RERUN_VERBOSELY=no PALUDIS_TESTS_KEEP_STDERR=yes \
            PALUDIS_TESTS_KEEP_LOG=yes TEST_OUTPUT_WRAPPER= $0 $@ > $out 2>&1
        echo ">>> saved output of verbose ${testname} rerun to ${out}" 1>&$PALUDIS_TESTS_REAL_STDOUT_FD
    fi
    echo ">>> exiting with error for test ${testname}" 1>&$PALUDIS_TESTS_REAL_STDOUT_FD
    exit 255
fi

if test -f "$TEST_SCRIPT_DIR""${testname}"_"cleanup.sh" ; then
    echo ">>> cleanup for test ${testname}" 1>&$PALUDIS_TESTS_REAL_STDOUT_FD
    if ! "$TEST_SCRIPT_DIR""${testname}"_"cleanup.sh" ; then
        echo ">>> exiting with error for test ${testname}" 1>&$PALUDIS_TESTS_REAL_STDOUT_FD
        exit 255
    fi
else
    echo ">>> No $TEST_SCRIPT_DIR${testname}_cleanup.sh to run" 1>&$PALUDIS_TESTS_REAL_STDOUT_FD
fi

echo ">>> exiting with success for test ${testname}" 1>&$PALUDIS_TESTS_REAL_STDOUT_FD
exit 0


