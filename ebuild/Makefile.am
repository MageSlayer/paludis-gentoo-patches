MAINTAINERCLEANFILES = Makefile.in
CLEANFILES = *~
SUBDIRS = . utils digests

libexecprogdir = $(libexecdir)/paludis/

libexecprog_SCRIPTS = \
	build_functions.bash \
	builtin_fetch.bash \
	builtin_init.bash \
	builtin_merge.bash \
	builtin_metadata.bash \
	builtin_strip.bash \
	builtin_tidyup.bash \
	builtin_unmerge.bash \
	ebuild.bash \
	echo_functions.bash \
	eclass_functions.bash \
	install_functions.bash \
	list_functions.bash \
	multilib_functions.bash \
	pkg_config.bash \
	pkg_nofetch.bash \
	pkg_postinst.bash \
	pkg_postrm.bash \
	pkg_preinst.bash \
	pkg_prerm.bash \
	pkg_setup.bash \
	portage_stubs.bash \
	sandbox_stubs.bash \
	src_compile.bash \
	src_install.bash \
	src_test.bash \
	src_unpack.bash \
	unpack_functions.bash \
	usage_error.bash

TESTS_ENVIRONMENT = env \
	PALUDIS_EBUILD_DIR="$(srcdir)/ebuild/" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	$(SHELL) $(top_srcdir)/ebuild/run_test.bash

TESTS = echo_functions_TEST.bash list_functions_TEST.bash
EXTRA_DIST = $(libexecprog_SCRIPTS) run_test.bash $(TESTS)

check_SCRIPTS = run_test.bash $(TESTS)
check_PROGRAMS =

