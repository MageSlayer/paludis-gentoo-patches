#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir broken_linkage_configuration_TEST_dir || exit 1
cd broken_linkage_configuration_TEST_dir || exit 1

mkdir -p etc/revdep-rebuild etc/ld.so.conf.d ld.so.conf.d

echo 'export PATH="/foobin:/barbin"' >>etc/profile.env
echo 'export ROOTPATH="/bazbin"' >>etc/profile.env
echo 'export WRATH="/goo"' >>etc/profile.env

echo '/foolib/bar' >>etc/ld.so.conf
echo '/barlib/foo' >>etc/ld.so.conf
echo '#/shoe' >>etc/ld.so.conf
echo '/meh=hahaha' >>etc/ld.so.conf
echo 'include ld.so.conf.d/*' >>etc/ld.so.conf
echo 'include /ld.so.conf.d/*' >>etc/ld.so.conf
echo 'include ld.so.conf.2' >>etc/ld.so.conf

echo '/qwerty1' >>etc/ld.so.conf.d/qwerty
echo '/qwerty2' >>etc/ld.so.conf.d/qwerty
echo '/uiop1' >>etc/ld.so.conf.d/uiop
echo '/uiop2' >>etc/ld.so.conf.d/uiop

echo '/fhqwhgads1' >>ld.so.conf.d/fhqwhgads
echo 'hwdep lalala' >>ld.so.conf.d/fhqwhgads
echo 'HWDEP lalalala' >>ld.so.conf.d/fhqwhgads
echo 'HwDep lalalalala' >>ld.so.conf.d/fhqwhgads
echo 'hwdp foobar' >>ld.so.conf.d/fhqwhgads
echo '/fhqwhgads2' >>ld.so.conf.d/fhqwhgads

echo '/42' >>etc/ld.so.conf.2

echo 'SEARCH_DIRS="/alib /blib"' >>etc/revdep-rebuild/10-test
echo 'SEARCH_DIRS_MASK="/meh -*"' >>etc/revdep-rebuild/10-test
echo 'SEARCH_DIRS_MASK="/feh"' >>etc/revdep-rebuild/20-test
echo 'LD_LIBRARY_MASK="libxyzzy.so"' >>etc/revdep-rebuild/30-test

echo 'SEARCH_DIRS="/moo"' >>etc/revdep-rebuild/10test~
echo 'SEARCH_DIRS="/foo"' >>etc/revdep-rebuild/.10test
echo 'SEARCH_DIRS="/boo"' >>etc/revdep-rebuild/\#10test

mkdir lib32 lib64


