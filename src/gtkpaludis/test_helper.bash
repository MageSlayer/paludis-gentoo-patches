#!/bin/bash

export XAUTHORITY=
unset LANG ${!LC*}

XDISPLAY=0
while [[ -f /tmp/.X${XDISPLAY}-lock ]] ; do
	XDISPLAY=$(( ${XDISPLAY} + 1 ))
done
export XDISPLAY=${XDISPLAY}
echo "Using X display :${XDISPLAY}" 1>&2

echo "Starting Xvfb..." 1>&2
Xvfb :${XDISPLAY} -screen 0 1024x768x24 &>/dev/null &

export DISPLAY=":${XDISPLAY}"
tries=0
while ! ./prod-x-server ; do
	sleep 1
	tries=$(( ${tries} + 1 ))

	if [[ ${tries} -gt 10 ]] ; then
		echo "Xvfb seems to be broken..." 1>&2
		kill $(</tmp/.X${XDISPLAY}-lock )
		exit 123
	fi
done

$@
exit_code=$?

echo "Killing Xvfb..." 1>&2
kill $(</tmp/.X${XDISPLAY}-lock )

exit $exit_code

