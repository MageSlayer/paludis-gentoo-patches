#!/bin/bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir output_wrapper_TEST_dir || exit 2
cd output_wrapper_TEST_dir

cat <<END > stdout_prefix.bash
echo one
echo two
echo three
END

cat <<END > stderr_prefix.bash
echo one 1>&2
echo two 1>&2
echo three 1>&2
END

cat <<END > mixed_prefix.bash
echo one 1>&2
echo two
echo three 1>&2
END

cat <<END > real_long_lines.bash
for (( a = 0 ; a < 1000 ; ++a )) ; do
    echo -n eeeeeeeeee 1>&2
    echo -n oooooooooo
done
echo 1>&2
echo
END

cat <<END > long_lines.bash
./outputwrapper --stdout-prefix 'o p ' --stderr-prefix 'e p ' -- \
    bash output_wrapper_TEST_dir/real_long_lines.bash \
    1>output_wrapper_TEST_dir/long_lines_stdout \
    2>output_wrapper_TEST_dir/long_lines_stderr
cat output_wrapper_TEST_dir/long_lines_stdout output_wrapper_TEST_dir/long_lines_stderr
END

cat <<END > real_no_trailing_newlines.bash
echo -n monkey
echo -n pants 1>&2
END

cat <<END > no_trailing_newlines.bash
./outputwrapper --stdout-prefix 'o p ' --stderr-prefix 'e p ' -- \
    bash output_wrapper_TEST_dir/real_no_trailing_newlines.bash \
    1>output_wrapper_TEST_dir/no_trailing_newlines_stdout \
    2>output_wrapper_TEST_dir/no_trailing_newlines_stderr
cat output_wrapper_TEST_dir/no_trailing_newlines_stdout output_wrapper_TEST_dir/no_trailing_newlines_stderr
END

cat <<END > exit_status.bash
echo lorem ipsum dolor
echo sit amet 1>&2
exit 5
END

cat <<END > no_wrap_blanks.bash
echo
echo one 1>&2
echo 1>&2
echo two
echo three 1>&2
echo 1>&2
echo
END

cat <<END > wrap_blanks.bash
echo
echo one 1>&2
echo 1>&2
echo two
echo three 1>&2
echo 1>&2
echo
END

cat <<END > discard_blank_output.bash
echo
echo 1>&2
echo
END

cat <<END > discard_blank_output_not_blank.bash
echo
echo 1>&2
echo
echo monkey
echo
END

cat <<END > discard_wrap_blank_output.bash
echo
echo 1>&2
echo
END

cat <<END > discard_wrap_blank_output_not_blank.bash
echo
echo 1>&2
echo
echo monkey
echo
END


