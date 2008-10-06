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
            description = Monkey
        :0 2
            description = Monkey
        :0 3
            description = Monkey
    pkg-two/
        :1 1
            description = Monkey
        :2 2
            description = Monkey
END

cd ..

