#!/usr/bin/env bash
# vim: set sw=4 sts=4 et tw=80 :

if test "xyes" = x"${BASH_VERSION}" ; then
    echo "This is not bash!"
    exit 127
fi

trap 'echo "exiting." ; exit 250' 15
KILL_PID=$$

misc/do_m4.bash paludis/Makefile.am || exit $?
misc/do_m4.bash paludis/paludis.hh || exit $?
misc/do_m4.bash paludis/util/util.hh || exit $?
misc/do_m4.bash paludis/util/Makefile.am || exit $?
misc/do_m4.bash hooks/Makefile.am || exit $?

