MAINTAINERCLEANFILES = Makefile.in
SUBDIRS = .

libexecprog3dir = $(libexecdir)/paludis/utils/3/

libexecprog3_SCRIPTS = \
	docompress \
	dohard \
	dosed \
	banned_in_eapi_3

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
EXTRA_DIST = \
	$(libexecprog3_SCRIPTS) \
	$(TESTS)

bannedscripts = dohard dosed

$(bannedscripts) : banned_in_eapi_3
	cat $? > $@
	chmod +x $@

CLEANFILES = *~ $(bannedscripts) *.epicfail

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

