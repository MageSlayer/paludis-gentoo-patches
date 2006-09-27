#!/bin/bash
# vim: set ft=sh sw=4 sts=4 et :

testname=${2:-${1}}
testname=${testname%.rb}

if test -f "$TEST_SCRIPT_DIR""${testname}"_"cleanup.sh" ; then
    echo ">>> cleanup for test ${testname}"
    if ! "$TEST_SCRIPT_DIR""${testname}"_"cleanup.sh" ; then
        echo ">>> exiting with error for test ${testname}"
        exit 255
    fi
fi

if test -f "$TEST_SCRIPT_DIR""${testname}"_"setup.sh" ; then
    echo ">>> setup for test ${testname}"
    if ! "$TEST_SCRIPT_DIR""${testname}"_"setup.sh" ; then
        echo ">>> exiting with error for test ${testname}"
        exit 255
    fi
fi

echo ">>> test ${testname}"
if ! ${@} ; then
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
fi

echo ">>> exiting with success for test ${testname}"
exit 0


