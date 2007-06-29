MAINTAINERCLEANFILES = Makefile.in
CLEANFILES = *~
SUBDIRS = . digests utils

AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@

libexecprogdir = $(libexecdir)/paludis/

libexecprog_SCRIPTS = \
	build_functions.bash \
	builtin_fetch.bash \
	builtin_fetchbin.bash \
	builtin_init.bash \
	builtin_initbin.bash \
	builtin_loadenv.bash \
	builtin_merge.bash \
	builtin_metadata.bash \
	builtin_prepare.bash \
	builtin_saveenv.bash \
	builtin_strip.bash \
	builtin_tidyup.bash \
	builtin_unmerge.bash \
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
	PALUDIS_EBUILD_LOG_LEVEL="warning" \
	TOP_BUILD_DIR="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/`" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	bash $(top_srcdir)/test/run_test.sh $(top_srcdir)/paludis/repositories/e/ebuild/run_test.bash

TESTS = echo_functions_TEST.bash list_functions_TEST.bash kernel_functions_TEST.bash \
	merge_TEST.bash unmerge_TEST.bash

libexecprog_PROGRAMS = merge unmerge
merge_SOURCES = merge.cc merge_common.cc merge_common.hh
merge_LDADD = \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/digests/libpaludisdigests.la \
	$(top_builddir)/paludis/selinux/libpaludisselinux.la \
	$(DYNAMIC_LD_LIBS) \
	$(PTHREAD_LIBS)
merge_LDFLAGS = $(PTHREAD_LIBS)

unmerge_SOURCES = unmerge.cc merge_common.cc merge_common.hh
unmerge_LDADD = \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/digests/libpaludisdigests.la \
	$(DYNAMIC_LD_LIBS) \
	$(PTHREAD_LIBS)
unmerge_LDFLAGS = $(PTHREAD_LIBS)

check_SCRIPTS = $(TESTS) \
	merge_TEST_setup.sh merge_TEST_cleanup.sh \
	unmerge_TEST_setup.sh unmerge_TEST_cleanup.sh \
	run_test.bash $(TESTS)

EXTRA_DIST = $(libexecprog_SCRIPTS) run_test.bash $(TESTS) $(check_SCRIPTS)

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done


