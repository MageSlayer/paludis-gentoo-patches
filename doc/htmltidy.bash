#!/usr/bin/env bash

if [[ "${1/.html}" == "${1}" ]] ; then
    echo "Not .html, no checking performed"
    exit 0
fi

tidy -utf8 -q < "${1}" 1>/dev/null

