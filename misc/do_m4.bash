#!/usr/bin/env bash
# vim: set sw=4 sts=4 et tw=80 :

if test "xyes" = x"${BASH_VERSION}" ; then
    echo "This is not bash!"
    exit 127
fi

trap 'echo "exiting." ; exit 250' 15
KILL_PID=$$

get_m4() {
    type "gm4" &>/dev/null && echo gm4 && return
    type "m4"  &>/dev/null && echo m4  && return
    echo "Could not find m4" 1>&2
    kill $KILL_PID
}

echo ">>> $(get_m4) -I. -I.. -I../.. -E ${1} > ${2}"
if ! $(get_m4) -I. -I.. -I../.. -E ${1} > ${2} ; then
    echo "oops!" 1>&2
    exit 127
fi
