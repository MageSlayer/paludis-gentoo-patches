#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir process_TEST_dir || exit 2
cd process_TEST_dir || exit 3

cat <<'END' > pipe_test.bash
#!/usr/bin/env bash

echo "$1" | tr "\n" "\0" 1>&$PALUDIS_PIPE_COMMAND_WRITE_FD
response1=
while true ; do
    read -n 1 -u $PALUDIS_PIPE_COMMAND_READ_FD c
    [[ "$c" == $'\0' ]] && break
    response1="${response1}${c}"
done

echo "$2" | tr "\n" "\0" 1>&$PALUDIS_PIPE_COMMAND_WRITE_FD
response2=
while true ; do
    read -n 1 -u $PALUDIS_PIPE_COMMAND_READ_FD c
    [[ "$c" == $'\0' ]] && break
    response2="${response2}${c}"
done

exit $response1$response2
END

cat <<'END' > captured_pipe_test.bash
#!/usr/bin/env bash

echo "$1" | tr "\n" "\0" 1>&$PALUDIS_PIPE_COMMAND_WRITE_FD
response1=
while true ; do
    read -n 1 -u $PALUDIS_PIPE_COMMAND_READ_FD c
    [[ "$c" == $'\0' ]] && break
    response1="${response1}${c}"
done

echo "$2" | tr "\n" "\0" 1>&$PALUDIS_PIPE_COMMAND_WRITE_FD
response2=
while true ; do
    read -n 1 -u $PALUDIS_PIPE_COMMAND_READ_FD c
    [[ "$c" == $'\0' ]] && break
    response2="${response2}${c}"
done

echo "$3" | tr "\n" "\0" 1>&$PALUDIS_PIPE_COMMAND_WRITE_FD
response3=
while true ; do
    read -n 1 -u $PALUDIS_PIPE_COMMAND_READ_FD c
    [[ "$c" == $'\0' ]] && break
    response3="${response3}${c}"
done

echo $response2

exit $response1$response3
END

