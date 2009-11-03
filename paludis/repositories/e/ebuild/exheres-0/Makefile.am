include $(top_srcdir)/misc/common-makefile.am

SUBDIRS = .

libexecprogexheresdir = $(libexecdir)/paludis/exheres-0

libexecprogexheres_SCRIPTS = \
	build_functions.bash \
	builtin_infovars.bash \
	builtin_init.bash \
	builtin_initrm.bash \
	builtin_initmisc.bash \
	builtin_loadenv.bash \
	builtin_metadata.bash \
	builtin_killold.bash \
	builtin_killoldrm.bash \
	builtin_saveenv.bash \
	builtin_tidyup.bash \
	builtin_tidyuprm.bash \
	builtin_variable.bash \
	conditional_functions.bash \
	ever_functions.bash \
	exlib_functions.bash \
	list_functions.bash \
	output_functions.bash \
	pkg_bad_options.bash \
	pkg_config.bash \
	pkg_info.bash \
	pkg_nofetch.bash \
	pkg_postinst.bash \
	pkg_postrm.bash \
	pkg_preinst.bash \
	pkg_prerm.bash \
	pkg_pretend.bash \
	pkg_setup.bash \
	portage_stubs.bash \
	src_compile.bash \
	src_configure.bash \
	src_fetch_extra.bash \
	src_install.bash \
	src_test.bash \
	src_test_expensive.bash \
	src_prepare.bash \
	src_unpack.bash

LOG_COMPILER_EXTRA = \
	PALUDIS_EBUILD_LOG_LEVEL="warning" \
	PALUDIS_EBUILD_MODULE_SUFFIXES="exheres-0"

LOG_COMPILER_INTERP = $(top_srcdir)/paludis/repositories/e/ebuild/run_test.bash

TESTS = conditional_functions_TEST.bash
check_SCRIPTS = $(TESTS)

EXTRA_DIST = $(libexecprogexheres_SCRIPTS) $(check_SCRIPTS)

