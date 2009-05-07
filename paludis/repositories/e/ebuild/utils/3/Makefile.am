MAINTAINERCLEANFILES = Makefile.in
SUBDIRS = .

libexecprog3dir = $(libexecdir)/paludis/utils/3/

libexecprog3_SCRIPTS = \
	docompress

TESTS_ENVIRONMENT = env \
	PALUDIS_EBUILD_DIR="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise \
			   $(top_srcdir)/paludis/repositories/e/ebuild/`" \
	PALUDIS_EBUILD_DIR_FALLBACK="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories/e/ebuild/`" \
	TOP_BUILD_DIR="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/`" \
	PALUDIS_EBUILD_LOG_LEVEL="silent" \
	PALUDIS_DISTRIBUTION="gentoo" \
	PALUDIS_OPTIONS="" \
	PALUDIS_NO_CHOWN="yes" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	PALUDIS_TMPDIR="`pwd`/" \
	bash $(top_srcdir)/paludis/repositories/e/ebuild/utils/run_test.bash

TESTS =
EXTRA_DIST = $(TESTS)

CLEANFILES = *~ *.epicfail

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

distcheck-deps-local : $(DISTCHECK_DEPS)

distcheck-deps : distcheck-deps-subdirs

distcheck-deps-subdirs :
	for s in $(SUBDIRS) . ; do if test x$$s = x. ; then $(MAKE) distcheck-deps-local || exit 1 ; \
	    else $(MAKE) -C $$s distcheck-deps || exit 1 ; fi ; done

