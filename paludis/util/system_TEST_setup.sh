#!/bin/bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir system_TEST_dir || exit 2
cd system_TEST_dir || exit 3

cat <<'END' > pipe_test.bash
#!/bin/bash

echo $1 1>&$PALUDIS_PIPE_COMMAND_WRITE_FD
read -u$PALUDIS_PIPE_COMMAND_READ_FD response1

echo $2 1>&$PALUDIS_PIPE_COMMAND_WRITE_FD
read -u$PALUDIS_PIPE_COMMAND_READ_FD response2

exit $response1$response2
END

cat <<'END' > captured_pipe_test.bash
#!/bin/bash

echo $1 1>&$PALUDIS_PIPE_COMMAND_WRITE_FD
read -u$PALUDIS_PIPE_COMMAND_READ_FD response1

echo $2 1>&$PALUDIS_PIPE_COMMAND_WRITE_FD
read -u$PALUDIS_PIPE_COMMAND_READ_FD response2

echo $3 1>&$PALUDIS_PIPE_COMMAND_WRITE_FD
read -u$PALUDIS_PIPE_COMMAND_READ_FD response3

echo $response2

exit $response1$response3
END

