#!/usr/bin/env bash
# vim: set sw=4 sts=4 ts=4 et tw=80 :

docker build -t paludis/exherbo-gcc exherbo/paludis-exherbo-gcc

docker build -t paludis/gentoo-gcc gentoo/paludis-gentoo-gcc

