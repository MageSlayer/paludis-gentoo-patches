#!/usr/bin/env bash
# vim: set sw=4 sts=4 ts=4 et tw=80 :

docker build \
    --no-cache \
    --rm \
    --pull \
    --tag paludis/exherbo-gcc \
    .
