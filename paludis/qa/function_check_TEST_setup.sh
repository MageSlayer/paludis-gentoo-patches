#!/bin/bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir function_check_TEST_dir || exit 2
cd function_check_TEST_dir || exit 3

mkdir "eclass" || exit 4

cat << EOE > eclass/with1.eclass || exit 5
function something () {
    : ;
}
EOE

cat << EOE > eclass/with2.eclass || exit 6
function foo
{
    : ;
}
EOE

cat << EOE > eclass/with3.eclass || exit 7
function       bar      (){
    : ;
}
EOE

cat << EOE > eclass/without.eclass || exit 8
# function something
bah() {
    : ;
}
EOE
