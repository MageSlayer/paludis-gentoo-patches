#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir broken_linkage_configuration_TEST_dir || exit 1
cd broken_linkage_configuration_TEST_dir || exit 1

mkdir -p etc/revdep-rebuild

echo 'export PATH="/foobin:/barbin"' >>etc/profile.env
echo 'export ROOTPATH="/bazbin"' >>etc/profile.env
echo 'export WRATH="/goo"' >>etc/profile.env

echo '/foolib/bar' >>etc/ld.so.conf
echo '/barlib/foo' >>etc/ld.so.conf
echo '#/shoe' >>etc/ld.so.conf

echo 'SEARCH_DIRS="/alib /blib"' >>etc/revdep-rebuild/10-test
echo 'SEARCH_DIRS_MASK="/meh -*"' >>etc/revdep-rebuild/10-test
echo 'SEARCH_DIRS_MASK="/feh"' >>etc/revdep-rebuild/20-test
echo 'LD_LIBRARY_MASK="libxyzzy.so"' >>etc/revdep-rebuild/30-test

echo 'SEARCH_DIRS="/moo"' >>etc/revdep-rebuild/10test~
echo 'SEARCH_DIRS="/foo"' >>etc/revdep-rebuild/.10test
echo 'SEARCH_DIRS="/boo"' >>etc/revdep-rebuild/\#10test

mkdir lib32 lib64


