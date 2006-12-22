CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda
DISTCLEANFILES = \
	portage_repository-sr.hh portage_repository-sr.cc \
	glsa-sr.hh glsa-sr.cc \
	portage_repository_params-sr.hh portage_repository_params-sr.cc
MAINTAINERCLEANFILES = Makefile.in

AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBDIR=\"$(libdir)\"

paludis_repositories_libdir = $(libdir)/paludis/repositories
paludis_repositories_lib_LTLIBRARIES = libpaludisportagerepository.la
paludis_repositories_portage_includedir = $(includedir)/paludis/repositories/portage/
libpaludisportagerepository_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0

paludis_repositories_portage_include_HEADERS = \
	glsa.hh \
	glsa-sr.hh \
	portage_repository-sr.hh \
	portage_repository.hh \
	portage_repository_profile_file.hh \
	portage_repository_profile.hh \
	portage_repository_exceptions.hh \
	portage_repository_news.hh \
	portage_repository_params.hh \
	portage_repository_params-sr.hh \
	portage_repository_sets.hh \
	portage_repository_entries.hh \
	ebuild_entries.hh \
	ebuild_flat_metadata_cache.hh \
	eclass_mtimes.hh \
	make_ebuild_repository.hh

libpaludisportagerepository_la_SOURCES = \
	glsa.cc \
	portage_repository.cc \
	portage_repository_profile_file.cc \
	portage_repository_profile.cc \
	portage_repository_exceptions.cc \
	portage_repository_news.cc \
	portage_repository_sets.cc \
	portage_repository_entries.cc \
	ebuild_entries.cc \
	ebuild_flat_metadata_cache.cc \
	eclass_mtimes.cc \
	portage_repository_params.cc \
	make_ebuild_repository.cc \
	$(paludis_repositories_portage_include_HEADERS)

libpaludisportagerepository_la_LIBADD = \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/libpaludis.la \
	$(DYNAMIC_LD_LIBS)

portage_repository_TEST_SOURCES = portage_repository_TEST.cc

portage_repository_TEST_LDADD = \
	libpaludisportagerepository.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/environment/test/libpaludistestenvironment.la \
	$(top_builddir)/test/libtest.a

portage_repository_TEST_CXXFLAGS = -I$(top_srcdir)

EXTRA_DIST = \
	glsa.sr \
	portage_repository_TEST.cc \
	portage_repository_TEST_setup.sh \
	portage_repository_TEST_cleanup.sh \
	portage_repository_params-sr.hh \
	portage_repository_params-sr.cc \
	portage_repository_params.sr \
	portage_repository-sr.hh \
	portage_repository-sr.cc \
	portage_repository.sr \
	xml_things_TEST.cc \
	xml_things_TEST_setup.sh \
	xml_things_TEST_cleanup.sh

BUILT_SOURCES = \
	portage_repository_params-sr.hh \
	portage_repository_params-sr.cc \
	portage_repository-sr.hh \
	portage_repository-sr.cc \
	glsa-sr.hh \
	glsa-sr.cc

check_SCRIPTS = \
	portage_repository_TEST_setup.sh portage_repository_TEST_cleanup.sh \
	xml_things_TEST_setup.sh xml_things_TEST_cleanup.sh

TESTS_ENVIRONMENT = env \
	PALUDIS_EBUILD_DIR="$(top_srcdir)/ebuild/" \
	PALUDIS_SKIP_CONFIG="yes" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	PALUDIS_REPOSITORY_SO_DIR="$(top_builddir)/paludis/repositories" \
	LD_LIBRARY_PATH="`$(top_srcdir)/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories/portage/`" \
	bash $(top_srcdir)/test/run_test.sh

portage_repository-sr.hh : portage_repository.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --header $(srcdir)/portage_repository.sr > $@

portage_repository-sr.cc : portage_repository.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --source $(srcdir)/portage_repository.sr > $@

portage_repository_params-sr.hh : portage_repository_params.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --header $(srcdir)/portage_repository_params.sr > $@

portage_repository_params-sr.cc : portage_repository_params.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --source $(srcdir)/portage_repository_params.sr > $@

glsa-sr.hh : glsa.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --header $(srcdir)/glsa.sr > $@

glsa-sr.cc : glsa.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --source $(srcdir)/glsa.sr > $@

if ENABLE_GLSA

if MONOLITHIC

noinst_LTLIBRARIES = libpaludisportagerepositoryxmlthings.la

else

lib_LTLIBRARIES = libpaludisportagerepositoryxmlthings.la

endif

libpaludisportagerepositoryxmlthings_la_SOURCES = xml_things.cc
libpaludisportagerepositoryxmlthings_la_CXXFLAGS = @LIBXML2DEPS_CFLAGS@
libpaludisportagerepositoryxmlthings_la_LIBADD = @LIBXML2DEPS_LIBS@ \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/libxml/libpaludislibxml.la
libpaludisportagerepositoryxmlthings_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0

GLSA_TESTS = xml_things_TEST
xml_things_TEST_SOURCES = xml_things_TEST.cc
xml_things_TEST_LDADD = \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/test/libtest.a \
	libpaludisportagerepository.la
endif

TESTS = portage_repository_TEST $(GLSA_TESTS)
check_PROGRAMS = $(TESTS)

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

