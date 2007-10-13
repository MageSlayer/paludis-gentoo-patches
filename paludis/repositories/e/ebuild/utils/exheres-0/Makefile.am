MAINTAINERCLEANFILES = Makefile.in
SUBDIRS = .

libexecprogdir = $(libexecdir)/paludis/utils/exheres-0

libexecprog_SCRIPTS = \
	dodoc \
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
	unpack \
	emake \
	banned_in_eapi_exheres-0

AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@

TESTS_ENVIRONMENT = env \
	PALUDIS_UTILITY_PATH_SUFFIXES="exheres-0" \
	PALUDIS_EBUILD_DIR="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise \
			   $(top_srcdir)/paludis/repositories/e/ebuild/`" \
	TOP_BUILD_DIR="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/`" \
	PALUDIS_EBUILD_LOG_LEVEL="silent" \
	PALUDIS_DISTRIBUTION="gentoo" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	bash $(top_srcdir)/paludis/repositories/e/ebuild/utils/run_test.bash

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

distcheck-deps : $(DISTCHECK_DEPS) distcheck-deps-subdirs

distcheck-deps-subdirs :
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s distcheck-deps || exit 1 ; done


