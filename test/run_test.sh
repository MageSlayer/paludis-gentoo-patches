#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

testname=${2:-${1}}
testname=${testname%.rb}
testname=${testname%.py}
testname=${testname%.bash}

TEST_OUTPUT_WRAPPER="${OVERRIDE_TEST_OUTPUT_WRAPPER-${TEST_OUTPUT_WRAPPER}}"

if test -f "$TEST_SCRIPT_DIR""${testname}"_"cleanup.sh" ; then
    echo ">>> cleanup for test ${testname}"
    if ! "$TEST_SCRIPT_DIR""${testname}"_"cleanup.sh" ; then
        echo ">>> exiting with error for test ${testname}"
        exit 255
    fi
else
    echo ">>> No $TEST_SCRIPT_DIR${testname}_cleanup.sh to run"
fi

if test -f "$TEST_SCRIPT_DIR""${testname}"_"setup.sh" ; then
    echo ">>> setup for test ${testname}"
    if ! "$TEST_SCRIPT_DIR""${testname}"_"setup.sh" ; then
        echo ">>> exiting with error for test ${testname}"
        exit 255
    fi
else
    echo ">>> No $TEST_SCRIPT_DIR${testname}_setup.sh to run"
fi

echo ">>> test ${testname}"
if [[ -n "${TEST_OUTPUT_WRAPPER}" ]] ; then
    $TEST_OUTPUT_WRAPPER --stdout-prefix "${testname#./}> " --stderr-prefix \
        "${testname#./}> " --wrap-blanks -- ${@}
    code=$?
else
    ${@}
    code=$?
fi

if [[ 0 != ${code} ]] ; then
    echo ">>> test ${testname} returned ${code}"
    if [[ -z "${PALUDIS_TESTS_RERUN_VERBOSELY}" ]] && [[ "${testname#./}" != "test_fail_TEST" ]] ; then
        out=`pwd`/${testname#./}.epicfail
        echo ">>> rerunning test ${testname} verbosely redirected to ${out}"
        env PALUDIS_TESTS_RERUN_VERBOSELY=no PALUDIS_TESTS_KEEP_STDERR=yes \
            PALUDIS_TESTS_KEEP_LOG=yes TEST_OUTPUT_WRAPPER= $0 $@ > $out 2>&1
        echo ">>> saved output of verbose ${testname} rerun to ${out}"
    fi
    echo ">>> exiting with error for test ${testname}"
    exit 255
fi

if test -f "$TEST_SCRIPT_DIR""${testname}"_"cleanup.sh" ; then
    echo ">>> cleanup for test ${testname}"
    if ! "$TEST_SCRIPT_DIR""${testname}"_"cleanup.sh" ; then
        echo ">>> exiting with error for test ${testname}"
        exit 255
    fi
else
    echo ">>> No $TEST_SCRIPT_DIR${testname}_cleanup.sh to run"
fi

echo ">>> exiting with success for test ${testname}"
exit 0


