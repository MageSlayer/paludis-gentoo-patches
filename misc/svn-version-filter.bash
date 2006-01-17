#!/usr/bin/env bash

# For Doxygen. See:
# http://www.stack.nl/~dimitri/doxygen/config.html#cfg_file_version_filter

source ${0/.bash/-data.bash}

case $1 in
    *.svn*)
        echo $VERSION
        ;;

    *)
        case "$(basename $1 )" in
            *.cc|*.hh|*.hh.in)
                if type svn &>/dev/null ; then
                    echo -n "svn "
                    svn stat -v $1 | sed -n 's/^[ A-Z?\*|!]\{1,15\}/r/;s/ \{1,15\}/\/r/;s/ .*//p'
                else
                    echo $VERSION
                fi
                ;;

            *)
                echo $VERSION
                ;;
        esac
        ;;
esac

