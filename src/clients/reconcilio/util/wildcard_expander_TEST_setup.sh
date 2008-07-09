#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir wildcard_expander_TEST_dir || exit 1
cd wildcard_expander_TEST_dir || exit 1

touch xyzzy xyz1zy xyz22zy
mkdir xyz123
touch xyz123/456zy

touch plugh

mkdir meh
touch meh/1 meh/2 meh/3 meh/44

touch foo123\* foo\*

touch fooAbar fooBbar fooCbar fooDbar fooEbar

