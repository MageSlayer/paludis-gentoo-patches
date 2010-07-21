#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir -p news_TEST_dir || exit 1
cd news_TEST_dir || exit 1

mkdir -p one/.paludis/repositories/ || exit 1

cat <<END > one/.paludis/repositories/one.conf
location = `pwd`/repo1
cache = /var/empty
format = e
names_cache = /var/empty
profiles = \${location}/profiles/testprofile
END

mkdir -p two/.paludis/repositories/ || exit 1

cat <<END > two/.paludis/repositories/one.conf
location = `pwd`/repo1
cache = /var/empty
format = e
names_cache = /var/empty
profiles = \${location}/profiles/testprofile
END

cat <<END > two/.paludis/repositories/two.conf
location = `pwd`/repo2
cache = /var/empty
format = e
names_cache = /var/empty
profiles = \${location}/profiles/testprofile
END

mkdir -p var/lib/gentoo/news
cat <<END > var/lib/gentoo/news/news-one.read
END
cat <<END > var/lib/gentoo/news/news-one.unread
END
cat <<END > var/lib/gentoo/news/news-two.read
END
cat <<END > var/lib/gentoo/news/news-two.unread
monkey
END

mkdir -p repo1/profiles/testprofile
echo one > repo1/profiles/repo_name

mkdir -p repo2/profiles/testprofile
echo two > repo2/profiles/repo_name

cd ..

