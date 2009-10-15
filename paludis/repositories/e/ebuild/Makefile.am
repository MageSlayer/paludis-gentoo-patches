MAINTAINERCLEANFILES = Makefile.in
CLEANFILES = *~ *.epicfail
SUBDIRS = . 0 1 2 3 exheres-0 kdebuild-1 paludis-1 pbin-1 utils

AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@

libexecprogdir = $(libexecdir)/paludis/

libexecprog_SCRIPTS = \
	binary_functions.bash \
	die_functions.bash \
	ebuild.bash \
	kernel_functions.bash \
	install_functions.bash \
	multilib_functions.bash \
	output_functions.bash \
	pipe_functions.bash \
	sandbox.bash \
	sydbox.bash \
	source_functions.bash \
	usage_error.bash \
	write_vdb_entry.bash \
	write_binary_ebuild.bash

TESTS_ENVIRONMENT = env \
	PALUDIS_EBUILD_DIR="$(top_srcdir)/paludis/repositories/e/ebuild/" \
	PALUDIS_EBUILD_DIR_FALLBACK="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories/e/ebuild/`" \
	PALUDIS_EXTRA_EBUILD_MODULES_DIRS="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/util/`" \
	PALUDIS_EBUILD_MODULE_SUFFIXES="0" \
	PALUDIS_EAPIS_DIR="$(top_srcdir)/paludis/eapis/" \
	PALUDIS_DISTRIBUTIONS_DIR="$(top_srcdir)/paludis/distributions/" \
	PALUDIS_DISTRIBUTION="gentoo" \
	PALUDIS_OPTIONS="" \
	PALUDIS_EBUILD_LOG_LEVEL="warning" \
	PALUDIS_DEFAULT_OUTPUT_CONF="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_srcdir)/paludis/environments/paludis/tests_output.conf`" \
	PALUDIS_OUTPUT_MANAGERS_DIR="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_srcdir)/paludis/environments/paludis/output_managers/`" \
	TOP_BUILD_DIR="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/`" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	PALUDIS_TMPDIR="`pwd`/" \
	bash $(top_srcdir)/test/run_test.sh $(top_srcdir)/paludis/repositories/e/ebuild/run_test.bash

TESTS = kernel_functions_TEST.bash

EXTRA_DIST = $(libexecprog_SCRIPTS) run_test.bash $(TESTS) $(check_SCRIPTS)

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

