MAINTAINERCLEANFILES = Makefile.in
CLEANFILES = *~
SUBDIRS = . 0 exheres-0 utils

AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@

libexecprogdir = $(libexecdir)/paludis/

libexecprog_SCRIPTS = \
	build_functions.bash \
	builtin_infovars.bash \
	builtin_init.bash \
	builtin_initbin.bash \
	builtin_loadenv.bash \
	builtin_metadata.bash \
	builtin_prepare.bash \
	builtin_saveenv.bash \
	builtin_strip.bash \
	builtin_tidyup.bash \
	builtin_unpackbin.bash \
	builtin_variable.bash \
	die_functions.bash \
	ebuild.bash \
	echo_functions.bash \
	kernel_functions.bash \
	eclass_functions.bash \
	exlib_functions.bash \
	install_functions.bash \
	list_functions.bash \
	multilib_functions.bash \
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
	sandbox.bash \
	src_compile.bash \
	src_install.bash \
	src_test.bash \
	src_unpack.bash \
	usage_error.bash \
	write_vdb_entry.bash \
	work_around_broken_utilities.bash

TESTS_ENVIRONMENT = env \
	PALUDIS_EBUILD_DIR="$(top_srcdir)/paludis/repositories/e/ebuild/" \
	PALUDIS_EAPIS_DIR="$(top_srcdir)/paludis/eapis/" \
	PALUDIS_DISTRIBUTIONS_DIR="$(top_srcdir)/paludis/distributions/" \
	PALUDIS_DISTRIBUTION="gentoo" \
	PALUDIS_EBUILD_LOG_LEVEL="warning" \
	TOP_BUILD_DIR="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/`" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	bash $(top_srcdir)/test/run_test.sh $(top_srcdir)/paludis/repositories/e/ebuild/run_test.bash

TESTS = echo_functions_TEST.bash list_functions_TEST.bash kernel_functions_TEST.bash

EXTRA_DIST = $(libexecprog_SCRIPTS) run_test.bash $(TESTS) $(check_SCRIPTS)

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

distcheck-deps-local : $(DISTCHECK_DEPS)

distcheck-deps : distcheck-deps-subdirs

distcheck-deps-subdirs :
	for s in $(SUBDIRS) . ; do if test x$$s = x. ; then make distcheck-deps-local || exit 1 ; \
	    else $(MAKE) -C $$s distcheck-deps || exit 1 ; fi ; done
