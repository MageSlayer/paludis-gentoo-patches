#!/bin/bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir cache_TEST_dir || exit 1
cd cache_TEST_dir || exit 1

cat <<END > entries
--- !ruby/object:Gem::Cache
gems:
  foo-1.2.3: !ruby/object:Gem::Specification
    name: foo
    version: !ruby/object:Gem::Version
      version: 1.2.3

  bar-2.3.4: !ruby/object:Gem::Specification
    name: bar
    version: !ruby/object:Gem::Version
      version: 2.3.4

END

cat <<"END" > broken
foo:
  [ [ foo:
END

