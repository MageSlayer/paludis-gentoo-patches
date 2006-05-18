#!/bin/bash
# vim: set ft=sh sw=4 sts=4 et :

if [ -d file_permissions_check_TEST_dir ] ; then
    chmod -R +rX file_permissions_check_TEST_dir
    rm -fr file_permissions_check_TEST_dir
else
    true
fi


