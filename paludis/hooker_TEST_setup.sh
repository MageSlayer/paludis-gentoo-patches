#!/bin/bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir hooker_TEST_dir || exit 2
cd hooker_TEST_dir || exit 3

mkdir simple_hook
cat <<"END" > simple_hook/one.bash
exit 3
END
chmod +x simple_hook/one.bash

mkdir fancy_hook
cat <<"END" > fancy_hook/one.hook
hook_run_fancy_hook() {
    return 5
}

hook_depend_fancy_hook() {
    echo
}

hook_after_fancy_hook() {
    echo
}
END
chmod +x fancy_hook/one.hook

mkdir several_hooks
cat <<"END" > several_hooks/one.hook
hook_run_several_hooks() {
    return 4
}

hook_depend_several_hooks() {
    echo
}
END
chmod +x several_hooks/one.hook

cat <<"END" > several_hooks/two.hook
hook_run_several_hooks() {
    return 6
}

hook_depend_several_hooks() {
    echo
}
END
chmod +x several_hooks/two.hook

cat <<"END" > several_hooks/three.hook
hook_run_several_hooks() {
    return 7
}

hook_depend_several_hooks() {
    echo
}
END
chmod +x several_hooks/three.hook

mkdir ordering
cat <<"END" > ordering.common
hook_run_ordering() {
    basename ${HOOK_FILE} | sed -e 's,\.hook$,,' >> hooker_TEST_dir/ordering.out
}

hook_depend_ordering() {
    case $(basename ${HOOK_FILE} | sed -e 's,\.hook$,,' ) in
        a)
        echo b
        ;;
        b)
        echo c d
        ;;
        c)
        echo e
        ;;
        d)
        echo e f
        ;;
        h)
    esac
}

hook_after_ordering() {
    case $(basename ${HOOK_FILE} | sed -e 's,\.hook$,,' ) in
        a)
        echo x
        ;;
        h)
        echo i
        ;;
        j)
        echo k
        ;;
    esac
}
END
chmod +x ordering.common

for a in a b c d e f g h i j k ; do
    ln -s ../ordering.common ordering/${a}.hook
done

