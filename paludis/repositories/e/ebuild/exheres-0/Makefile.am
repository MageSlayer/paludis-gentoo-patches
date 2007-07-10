MAINTAINERCLEANFILES = Makefile.in
CLEANFILES = *~
SUBDIRS = .

libexecprogexheresdir = $(libexecdir)/paludis/exheres-0

libexecprogexheres_SCRIPTS = \
	builtin_fetch.bash \
	pkg_nofetch.bash \
	list_functions.bash \
	echo_functions.bash

TESTS_ENVIRONMENT = env \
	PALUDIS_EBUILD_DIR="$(top_srcdir)/paludis/repositories/e/ebuild/" \
	PALUDIS_EAPIS_DIR="$(top_srcdir)/paludis/eapis/" \
	PALUDIS_DISTRIBUTIONS_DIR="$(top_srcdir)/paludis/distributions/" \
	PALUDIS_EBUILD_LOG_LEVEL="warning" \
	TOP_BUILD_DIR="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/`" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	bash $(top_srcdir)/test/run_test.sh $(top_srcdir)/paludis/repositories/e/ebuild/run_test.bash

TESTS =
check_SCRIPTS = $(TESTS)

EXTRA_DIST = $(libexecprogexheres_SCRIPTS)

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

