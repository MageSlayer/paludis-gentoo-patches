MAINTAINERCLEANFILES = Makefile.in
CLEANFILES = *~ *.epicfail
SUBDIRS = .

libexecprog2dir = $(libexecdir)/paludis/2

libexecprog2_SCRIPTS = \
	eclass_functions.bash \
	pkg_nofetch.bash \
	src_compile.bash \
	src_configure.bash \
	src_install.bash \
	src_prepare.bash \
	src_test.bash \
	src_unpack.bash \
	output_functions.bash

TESTS_ENVIRONMENT = env \
	PALUDIS_EBUILD_DIR="$(top_srcdir)/paludis/repositories/e/ebuild/" \
	PALUDIS_EBUILD_DIR_FALLBACK="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories/e/ebuild/`" \
	PALUDIS_EBUILD_MODULE_SUFFIXES="1 0" \
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

TESTS =
check_SCRIPTS = $(TESTS)

EXTRA_DIST = $(libexecprog2_SCRIPTS)

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

