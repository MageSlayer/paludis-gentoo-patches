#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir fs_merger_TEST_dir || exit 2
cd fs_merger_TEST_dir || exit 3

# must be before anything else, since timestamps before this are treated as
# dodgy
touch reference

mkdir -p sym_over_nothing_dir/{image,root}
ln -s image_dst sym_over_nothing_dir/image/sym
ln -s ${PWD}/sym_over_nothing_dir/image/rewrite_target sym_over_nothing_dir/image/rewrite_me

mkdir -p sym_over_sym_dir/{image,root}
ln -s image_dst sym_over_sym_dir/image/sym
ln -s root_dst sym_over_sym_dir/root/sym
ln -s ${PWD}/sym_over_sym_dir/image/rewrite_target sym_over_sym_dir/image/rewrite_me
ln -s rewrite_target sym_over_sym_dir/root/rewrite_me

mkdir -p sym_over_file_dir/{image,root}
ln -s image_dst sym_over_file_dir/image/sym
> sym_over_file_dir/root/sym
ln -s ${PWD}/sym_over_file_dir/image/rewrite_target sym_over_file_dir/image/rewrite_me
> sym_over_file_dir/root/rewrite_target

mkdir -p sym_over_dir_dir/{image,root}
ln -s image_dst sym_over_dir_dir/image/sym
mkdir sym_over_dir_dir/root/sym


mkdir -p dir_over_nothing_dir/{image,root}
mkdir dir_over_nothing_dir/image/dir

mkdir -p dir_over_dir_dir/{image,root}
mkdir dir_over_dir_dir/image/dir
mkdir dir_over_dir_dir/root/dir

mkdir -p dir_over_file_dir/{image,root}
mkdir dir_over_file_dir/image/dir
> dir_over_file_dir/root/dir

mkdir -p dir_over_sym_1_dir/{image,root}
mkdir dir_over_sym_1_dir/image/dir
> dir_over_sym_1_dir/image/dir/file
mkdir dir_over_sym_1_dir/root/realdir
ln -s realdir dir_over_sym_1_dir/root/dir

mkdir -p dir_over_sym_2_dir/{image,root}
mkdir dir_over_sym_2_dir/image/dir
> dir_over_sym_2_dir/image/dir/file
> dir_over_sym_2_dir/root/file
ln -s file dir_over_sym_2_dir/root/dir

mkdir -p dir_over_sym_3_dir/{image,root}
mkdir dir_over_sym_3_dir/image/dir
> dir_over_sym_3_dir/image/dir/file
ln -s nowhere dir_over_sym_3_dir/root/dir


mkdir -p file_over_nothing_dir/{image,root}
echo "image contents" > file_over_nothing_dir/image/file

mkdir -p file_over_file_dir/{image,root}
echo "image contents" > file_over_file_dir/image/file
echo "root contents" > file_over_file_dir/root/file

mkdir -p file_over_sym_dir/{image,root}
echo "image 1 contents" > file_over_sym_dir/image/file1
echo "image 2 contents" > file_over_sym_dir/image/file2
echo "image 3 contents" > file_over_sym_dir/image/file3
ln -s nowhere file_over_sym_dir/root/file1
ln -s file file_over_sym_dir/root/file2
ln -s dir file_over_sym_dir/root/file3
> file_over_sym_dir/root/file
mkdir file_over_sym_dir/root/dir

mkdir -p file_over_dir_dir/{image,root}
> file_over_dir_dir/image/file
mkdir file_over_dir_dir/root/file

mkdir -p override_dir/{image,root}
mkdir override_dir/image/dir_skip_me/
mkdir override_dir/image/dir_install_me/
> override_dir/image/file_skip_me
> override_dir/image/file_install_me
ln -s override_dir/image/file_skip_me override_dir/image/sym_skip_me
ln -s override_dir/image/file_install_me override_dir/image/sym_install_me

mkdir -p empty_{dir,root}_{allowed,disallowed}_dir/{image,root}
mkdir -p empty_dir_{allowed,disallowed}_dir/image/empty

for d in *_dir; do
    ln -s ${d} ${d%_dir}
done

mkdir -p mtimes/{image/dir,root}
> mtimes/image/new_file
> mtimes/image/existing_file
touch -d '3 years ago' mtimes/image/dodgy_file
> mtimes/image/dir/new_file
touch -d '3 years ago' mtimes/image/dir/dodgy_file
> mtimes/root/existing_file

mkdir -p mtimes_fix/{image/dir,root}
> mtimes_fix/image/new_file
> mtimes_fix/image/existing_file
touch -d '3 years ago' mtimes_fix/image/dodgy_file
> mtimes_fix/image/dir/new_file
touch -d '3 years ago' mtimes_fix/image/dir/dodgy_file
> mtimes_fix/root/existing_file

mkdir hooks
cd hooks
mkdir \
merger_install_file_override \
merger_install_sym_override \
merger_install_dir_override

cat <<"END" > universal_override.hook
hook_run_merger_install_file_override() {
    if [[ "${INSTALL_DESTINATION}" == *"/file_skip_me" ]]; then
        echo "skip"
    fi
}

hook_run_merger_install_sym_override() {
    if [[ "${INSTALL_DESTINATION}" == *"/sym_skip_me" ]]; then
        echo "skip"
    fi
}

hook_run_merger_install_dir_override() {
    if [[ "${INSTALL_DESTINATION}" == *"/dir_skip_me" ]]; then
        echo "skip"
    fi
}
END
chmod +x universal_override.hook
for dir in merger_install_*_override; do
    ln -s ../universal_override.hook  ${dir}
done
