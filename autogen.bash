#!/usr/bin/env bash
# vim: set sw=4 sts=4 et tw=80 :

if test "xyes" = x"${BASH_VERSION}" ; then
    echo "This is not bash!"
    exit 127
fi

trap 'echo "exiting." ; exit 250' 15
KILL_PID=$$

run() {
    echo ">>> $@" 1>&2
    if ! $@ ; then
        echo "oops!" 1>&2
        exit 127
    fi
}

get() {
    type ${1}-${2}    &>/dev/null && echo ${1}-${2}    && return
    type ${1}${2//.}  &>/dev/null && echo ${1}${2//.}  && return
    type ${1}         &>/dev/null && echo ${1}         && return
    echo "Could not find ${1} ${2}" 1>&2
    kill $KILL_PID
}

misc/do_m4.bash paludis/Makefile.am || exit $?
misc/do_m4.bash paludis/paludis.hh || exit $?
misc/do_m4.bash paludis/qa/Makefile.am || exit $?
misc/do_m4.bash paludis/qa/qa.hh || exit $?
misc/do_m4.bash paludis/smart_record.hh || exit $?
misc/do_m4.bash paludis/comparison_policy.hh || exit $?
run mkdir -p config
run $(get libtoolize 1.5 ) --copy --force --automake
rm -f config.cache
run $(get aclocal 1.9 )
run $(get autoheader 2.59 )
run $(get autoconf 2.59 )
run $(get automake 1.9 ) -a --copy

