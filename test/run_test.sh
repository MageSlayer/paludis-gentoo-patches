#!/bin/bash
# vim: set ft=sh sw=4 sts=4 et :

testname=${2:-${1}}
testname=${testname%.rb}
testname=${testname%.py}
testname=${testname%.bash}

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
    $TEST_OUTPUT_WRAPPER --stdout-prefix "${testname#./}> " --stderr-prefix "${testname#./}> " -- ${@}
    code=$?
else
    ${@}
    code=$?
fi

if [[ 0 != ${code} ]] ; then
    echo ">>> test ${testname} returned ${code}"
    if test -f "$TEST_SCRIPT_DIR""${testname}"_"cleanup.sh" ; then
        echo ">>> cleanup for test ${testname}"
        "$TEST_SCRIPT_DIR""${testname}"_"cleanup.sh"
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


