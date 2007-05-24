#!/bin/bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir merger_TEST_dir || exit 2
cd merger_TEST_dir || exit 3


mkdir image_sym_over_nothing root_sym_over_nothing
ln -s image_dst image_sym_over_nothing/sym

mkdir image_sym_over_sym root_sym_over_sym
ln -s image_dst image_sym_over_sym/sym
ln -s root_dst root_sym_over_sym/sym

mkdir image_sym_over_file root_sym_over_file
ln -s image_dst image_sym_over_file/sym
> root_sym_over_file/sym

mkdir image_sym_over_dir root_sym_over_dir
ln -s image_dst image_sym_over_dir/sym
mkdir root_sym_over_dir/sym


mkdir image_dir_over_nothing root_dir_over_nothing
mkdir image_dir_over_nothing/dir

mkdir image_dir_over_dir root_dir_over_dir
mkdir image_dir_over_dir/dir
mkdir root_dir_over_dir/dir

mkdir image_dir_over_file root_dir_over_file
mkdir image_dir_over_file/dir
> root_dir_over_file/dir

mkdir image_dir_over_sym_1 root_dir_over_sym_1
mkdir image_dir_over_sym_1/dir
> image_dir_over_sym_1/dir/file
mkdir root_dir_over_sym_1/realdir
ln -s realdir root_dir_over_sym_1/dir

mkdir image_dir_over_sym_2 root_dir_over_sym_2
mkdir image_dir_over_sym_2/dir
> image_dir_over_sym_2/dir/file
> root_dir_over_sym_2/file
ln -s file root_dir_over_sym_2/dir

mkdir image_dir_over_sym_3 root_dir_over_sym_3
mkdir image_dir_over_sym_3/dir
> image_dir_over_sym_3/dir/file
ln -s nowhere root_dir_over_sym_3/dir


mkdir image_file_over_nothing root_file_over_nothing
echo "image contents" > image_file_over_nothing/file

mkdir image_file_over_file root_file_over_file
echo "image contents" > image_file_over_file/file
echo "root contents" > root_file_over_file/file

mkdir image_file_over_sym root_file_over_sym
echo "image 1 contents" > image_file_over_sym/file1
echo "image 2 contents" > image_file_over_sym/file2
echo "image 3 contents" > image_file_over_sym/file3
ln -s nowhere root_file_over_sym/file1
ln -s file root_file_over_sym/file2
ln -s dir root_file_over_sym/file3
> root_file_over_sym/file
mkdir root_file_over_sym/dir

mkdir image_file_over_dir root_file_over_dir
> image_file_over_dir/file
mkdir root_file_over_dir/file

mkdir image_override root_override
mkdir image_override/dir_skip_me/
mkdir image_override/dir_install_me/
> image_override/file_skip_me
> image_override/file_install_me
ln -s image_override/file_skip_me image_override/sym_skip_me
ln -s image_override/file_install_me image_override/sym_install_me


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
