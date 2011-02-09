#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir unwritten_repository_TEST_dir || exit 1
cd unwritten_repository_TEST_dir || exit 1

mkdir -p repo1
cat <<"END" > repo1/foo.conf
format = unwritten-1
END

mkdir -p repo2
cat <<"END" > repo2/foo.conf
format = unwritten-1

cat-one/
    pkg-one/
        :0 1
            description = Description for pkg-one-1:0
        :0 2
            description = Description for pkg-one-2:0
        :0 3
            description = Description for pkg-one-3:0
    pkg-two/
        :1 1
            description = Description for pkg-two-1:1
        :2 2
            description = Description for pkg-two-2:2
    pkg-three/
        :1 1 2 3
            description = Description for pkg-three:1
        :2 4 5 6
            description = Description for pkg-three:2
END

cd ..

