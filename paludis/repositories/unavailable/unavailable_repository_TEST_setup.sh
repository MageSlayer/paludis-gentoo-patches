#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir unavailable_repository_TEST_dir || exit 1
cd unavailable_repository_TEST_dir || exit 1

mkdir -p repo1
cat <<"END" > repo1/foo.repository
END

mkdir -p repo2
cat <<"END" > repo2/foo.repository
format = unavailable-2
repo_name = foo

cat-one/
    pkg-one/
        :0 1 2 3 ; Monkey
    pkg-two/
        :1 1 ; One
        :2 2 ; Two
END
cat <<"END" > repo2/bar.repository
format = unavailable-2
repo_name = bar

cat-one/
    pkg-one/
        :0 1 ; Chimp
        :3 3 ; Banana
    pkg-six/
        :0 3 ; Three
cat-two/
    pkg-six/
        :0 1 ; Cheese on toast
END

cd ..

