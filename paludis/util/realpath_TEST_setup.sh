#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir realpath_TEST_dir || exit 1
cd realpath_TEST_dir || exit 1

mkdir -p usr/lib64
ln -s lib64 usr/lib

touch usr/lib64/libfoo.so.1
ln -s libfoo.so.1 usr/lib64/libfoo.so

touch usr/lib64/libbar.so.1
ln -s ../lib64/libbar.so.1 usr/lib64/libbar.so

touch usr/lib64/libbaz.so.1
ln -s /usr/lib64/libbaz.so.1 usr/lib64/libbaz.so

touch usr/lib64/libxyzzy.so.1
ln -s /usr/lib/libxyzzy.so.1 usr/lib64/libxyzzy.so

touch usr/lib64/libplugh.so.1
ln -s ./libplugh.so.1 usr/lib64/libplugh.so

touch usr/lib64/libplover.so.1
ln -s /usr/lib/../lib64/libplover.so.1 usr/lib64/libplover.so

touch usr/lib64/libblast.so.1
ln -s ../lib64/./libblast.so.1 usr/lib64/libblast.so

touch usr/lib64/libquux.so.1
ln -s //usr/lib64/libquux.so.1 usr/lib64/libquux.so

touch usr/lib64/libnarf.so.1
ln -s ../lib64//libnarf.so.1 usr/lib64/libnarf.so

touch usr/lib64/libblech.so.1
ln -s ../../../../../../../../../../usr/lib/libblech.so.1 usr/lib64/libblech.so

ln -s libstab.so.1 usr/lib64/libstab.so

ln -s /usr/lib/libsnark.so.1 usr/lib64/libsnark.so

ln -s /usr/lib32/libfool.so.1 usr/lib64/libfool.so

ln -s ../lib/barf/libbarf.so.1 usr/lib64/libbarf.so

touch usr/lib64/libblip.so.1.0.1
ln -s libblip.so.1.0.1 usr/lib64/libblip.so.1
ln -s libblip.so.1 usr/lib64/libblip.so

touch usr/lib64/libpoing.so.1.0.1
ln -s /usr/lib64/libpoing.so.1.0.1 usr/lib64/libpoing.so.1
ln -s /usr/lib/libpoing.so.1 usr/lib64/libpoing.so

mkdir usr/lib64/x
touch usr/lib64/x/liby.so.1
ln -s /usr/lib64/x/liby.so.1 usr/lib64/x/liby.so

ln -s libouch.so usr/lib64/libouch.so

ln -s libping.so usr/lib64/libpong.so
ln -s libpong.so usr/lib64/libping.so

