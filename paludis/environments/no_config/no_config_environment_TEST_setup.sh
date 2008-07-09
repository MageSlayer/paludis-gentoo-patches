#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir no_config_environment_TEST_dir || exit 2
cd no_config_environment_TEST_dir || exit 3

mkdir -p repo/{profiles/profile,cat-one/pkg-one}
cat <<END > repo/profiles/repo_name
foo
END
cat <<END > repo/profiles/categories
cat-one
END
cat <<END > repo/profiles/profiles.desc
foo profile stable
END
cat <<END > repo/profiles/profile/make.defaults
ARCH="foo"
USE="moo"
USE_EXPAND="EXP MORE_EXP THIRD_EXP"
EXP="one"
MORE_EXP="one"
THIRD_EXP="one"
END

