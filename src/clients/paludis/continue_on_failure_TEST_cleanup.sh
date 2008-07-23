#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

if [ -d continue_on_failure_TEST_dir ] ; then
    rm -fr continue_on_failure_TEST_dir
else
    true
fi

