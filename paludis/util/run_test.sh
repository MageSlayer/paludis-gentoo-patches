#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

testname=$(basename ${2:-${1}})
interp=
[[ ${testname%.py} != ${testname} ]] && interp="${PYTHON}"
[[ ${testname%.rb} != ${testname} ]] && interp="${RUBY} -I ./.libs/"

testname=${testname%.rb}
testname=${testname%.py}
testname=${testname%.bash}

export PALUDIS_TEST_PROGRAM=$testname

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

now=$(date +'%s' )
echo ">>> test ${testname}"
$interp ${@}
code=$?
duration=$(( $(date +'%s' ) - ${now} ))

if [[ 0 != ${code} ]] ; then
    echo ">>> test ${testname} returned ${code} [${duration}s]"
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

echo ">>> exiting with success for test ${testname} [${duration}s]"
exit 0


