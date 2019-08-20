#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir hooker_TEST_dir || exit 2
cd hooker_TEST_dir || exit 3

mkdir simple_hook
cat <<"END" > simple_hook/one.bash
exit 3
END
chmod +x simple_hook/one.bash

mkdir simple_hook_output
cat <<"END" > simple_hook_output/one.bash
echo "foo"
END
chmod +x simple_hook_output/one.bash


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

mkdir fancy_hook_output
cat <<"END" > fancy_hook_output/one.hook
hook_run_fancy_hook_output() {
    echo foo
}

hook_depend_fancy_hook_output() {
    echo
}

hook_after_fancy_hook_output() {
    echo
}
END
chmod +x fancy_hook_output/one.hook


mkdir so_hook
ln -s ${TOP_BUILDDIR}/paludis/libpaludissohooks_TEST_${PALUDIS_PC_SLOT}.so.${SO_SUFFIX} so_hook

mkdir so_hook_output
ln -s ${TOP_BUILDDIR}/paludis/libpaludissohooks_TEST_${PALUDIS_PC_SLOT}.so.${SO_SUFFIX} so_hook_output

mkdir py_hook

cat <<"END" > py_hook/hook.py
def hook_run_py_hook(env, hook_env):
    pass
END

mkdir py_hook_output
cat <<"END" > py_hook_output/hook.py
def hook_run_py_hook_output(env, hook_env):
    return "foo"
END

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

mkdir several_hooks_output
cat <<"END" > several_hooks_output/one.hook
hook_run_several_hooks_output() {
    echo one
    echo one >> hooker_TEST_dir/several_output.out
}

hook_depend_several_hooks_output() {
    echo
}
END
chmod +x several_hooks_output/one.hook

cat <<"END" > several_hooks_output/two.hook
hook_run_several_hooks_output() {
    echo two
    echo two >> hooker_TEST_dir/several_output.out
}

hook_depend_several_hooks_output() {
    echo one
}
END
chmod +x several_hooks_output/two.hook

cat <<"END" > several_hooks_output/three.hook
hook_run_several_hooks_output() {
    echo three
    echo three >> hooker_TEST_dir/several_output.out
}

hook_depend_several_hooks_output() {
    echo two
}
END
chmod +x several_hooks_output/three.hook

mkdir several_hooks_output_bad
cat <<"END" > several_hooks_output_bad/one.hook
hook_run_several_hooks_output_bad() {
    echo one
    echo one >> hooker_TEST_dir/several_output_bad.out
    return 1
}

hook_depend_several_hooks_output_bad() {
    echo
}
END
chmod +x several_hooks_output_bad/one.hook

cat <<"END" > several_hooks_output_bad/two.hook
hook_run_several_hooks_output_bad() {
    echo two
    echo two >> hooker_TEST_dir/several_output_bad.out
    return 99
}

hook_depend_several_hooks_output_bad() {
    echo one
}
END
chmod +x several_hooks_output_bad/two.hook

cat <<"END" > several_hooks_output_bad/three.hook
hook_run_several_hooks_output_bad() {
    echo three
    echo three >> hooker_TEST_dir/several_output_bad.out
    return 2
}

hook_depend_several_hooks_output_bad() {
    echo two
}
END
chmod +x several_hooks_output_bad/three.hook

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
        echo i py_hook
        ;;
        j)
        echo k libpaludissohooks_TEST
        ;;
    esac
}
END
chmod +x ordering.common

for a in a b c d e f g h i j k ; do
    ln -s ../ordering.common ordering/${a}.hook
done
ln -s ${TOP_BUILDDIR}/paludis/libpaludissohooks_TEST_${PALUDIS_PC_SLOT}.so.${SO_SUFFIX} ordering
cat <<"END" > ordering/py_hook.py
def hook_run_ordering(env, hook_env):
    open("hooker_TEST_dir/ordering.out", "a").write("py_hook\n")
def hook_depend_ordering(hook_env):
    return ["f"]
END

mkdir bad_hooks
cat <<"END" > bad_hooks.common
hook_run_bad_hooks() {
    basename ${HOOK_FILE} | sed -e 's,\.hook$,,' >> hooker_TEST_dir/bad_hooks.out
}
END
chmod +x bad_hooks.common

for a in one three ; do
    ln -s ../bad_hooks.common bad_hooks/${a}.hook
done

cat <<"END" > bad_hooks/two.hook
asdf
END
chmod +x bad_hooks/two.hook

cat <<"END" > bad_hooks/four.py
def hook_run_bad_hooks(env, hook_env):
    1/0
END

mkdir cycles
cat <<"END" > cycles.common
hook_run_cycles() {
    basename ${HOOK_FILE} | sed -e 's,\.hook$,,' >> hooker_TEST_dir/cycles.out
}

hook_depend_cycles() {
    case $(basename ${HOOK_FILE} | sed -e 's,\.hook$,,' ) in
        c)
        echo d;
        ;;
        d)
        echo e;
        ;;
        e)
        echo c;
        ;;
        f)
        echo g;
        ;;
        h)
        echo h;
        ;;
    esac
}

hook_after_cycles() {
    case $(basename ${HOOK_FILE} | sed -e 's,\.hook$,,' ) in
        a)
        echo b
        ;;
    esac
}
END
chmod +x cycles.common

for a in a b c d e f g h i ; do
    ln -s ../cycles.common cycles/${a}.hook
done

