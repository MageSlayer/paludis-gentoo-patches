MAINTAINERCLEANFILES = Makefile.in
SUBDIRS = .

AUTOMAKE_OPTIONS = parallel-tests

libexecprogdir = $(libexecdir)/paludis/utils/kdebuild-1

libexecprog_SCRIPTS = \
	dohard \
	dohtml \
	dosed \
	donewins \
	prepall \
	prepallstrip \
	prepstrip \
	prepallman \
	prepman \
	prepallinfo \
	prepinfo \
	prepdocs \
	prepalldocs \
	ecompress \
	ecompressdir \
	banned_in_eapi_kdebuild-1

AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@

TESTS_ENVIRONMENT = env \
	PALUDIS_UTILITY_PATH_SUFFIXES="exheres-0" \
	PALUDIS_EBUILD_DIR="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise \
			   $(top_srcdir)/paludis/repositories/e/ebuild/`" \
	PALUDIS_EBUILD_DIR_FALLBACK="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories/e/ebuild/`" \
	TOP_BUILD_DIR="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/`" \
	PALUDIS_EBUILD_LOG_LEVEL="silent" \
	PALUDIS_DISTRIBUTION="gentoo" \
	PALUDIS_OPTIONS="" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	bash $(top_srcdir)/paludis/repositories/e/ebuild/utils/run_test.bash

TESTS =

EXTRA_DIST = $(libexecprog_SCRIPTS) $(check_SCRIPTS) \
	banned_in_eapi_kdebuild-1 $(TESTS)

bannedscripts = prepall prepallstrip prepstrip prepallman prepman prepallinfo \
		prepinfo prepdocs prepalldocs dohard donewins dosed dohtml \
		ecompress ecompressdir

CLEANFILES = *~ $(bannedscripts) *.epicfail

$(bannedscripts) : banned_in_eapi_kdebuild-1
	cat $? > $@
	chmod +x $@

check_PROGRAMS =

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

