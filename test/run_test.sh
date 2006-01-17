#!/bin/sh
# vim: set ft=sh sw=4 sts=4 et :

if test -f "$TEST_SCRIPT_DIR""$1"_"setup.sh" ; then
    echo ">>> setup for test $1"
    if ! "$TEST_SCRIPT_DIR""$1"_"setup.sh" ; then
        echo ">>> exiting with error for test $1"
        exit 255
    fi
fi

echo ">>> test $1"
if ! $1 ; then
    if test -f "$TEST_SCRIPT_DIR""$1"_"cleanup.sh" ; then
        echo ">>> cleanup for test $1"
        "$TEST_SCRIPT_DIR""$1"_"cleanup.sh"
    fi
    echo ">>> exiting with error for test $1"
    exit 255
fi

if test -f "$TEST_SCRIPT_DIR""$1"_"cleanup.sh" ; then
    echo ">>> cleanup for test $1"
    if ! "$TEST_SCRIPT_DIR""$1"_"cleanup.sh" ; then
        echo ">>> exiting with error for test $1"
        exit 255
    fi
fi

echo ">>> exiting with success for test $1"
exit 0


