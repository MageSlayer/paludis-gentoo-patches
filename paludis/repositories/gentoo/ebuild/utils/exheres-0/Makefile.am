MAINTAINERCLEANFILES = Makefile.in
SUBDIRS = .

libexecprogdir = $(libexecdir)/paludis/utils/exheres-0

libexecprog_SCRIPTS = \
	dohard \
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
	banned_in_eapi_exheres-0

AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@

TESTS_ENVIRONMENT = env \
	PALUDIS_UTILITY_PATH_SUFFIXES="exheres-0" \
	PALUDIS_EBUILD_DIR="`$(top_srcdir)/paludis/repositories/gentoo/ebuild/utils/canonicalise \
			   $(top_srcdir)/paludis/repositories/gentoo/ebuild/`" \
	TOP_BUILD_DIR="`$(top_srcdir)/paludis/repositories/gentoo/ebuild/utils/canonicalise $(top_builddir)/`" \
	PALUDIS_EBUILD_LOG_LEVEL="silent" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	bash $(top_srcdir)/paludis/repositories/gentoo/ebuild/utils/run_test.bash

TESTS =

EXTRA_DIST = $(libexecprog_SCRIPTS) $(check_SCRIPTS) \
	banned_in_eapi_exheres-0 $(TESTS)

bannedscripts = prepall prepallstrip prepstrip prepallman prepman prepallinfo \
		prepinfo prepdocs prepalldocs dohard donewins dosed

CLEANFILES = *~ $(bannedscripts)

$(bannedscripts) : banned_in_eapi_exheres-0
	cat $? > $@

check_PROGRAMS =

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

