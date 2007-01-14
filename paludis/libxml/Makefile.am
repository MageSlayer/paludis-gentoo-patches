CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda
MAINTAINERCLEANFILES = Makefile.in
AM_CXXFLAGS = -I$(top_srcdir) -I$(top_builddir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\"
SUBDIRS = .

TESTS =

TESTS_ENVIRONMENT = env \
	PALUDIS_EBUILD_DIR="$(top_srcdir)/ebuild/" \
	PALUDIS_SKIP_CONFIG="yes" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	bash $(top_srcdir)/test/run_test.sh

check_PROGRAMS = $(TESTS)
check_SCRIPTS =

if NEED_XML

if MONOLITHIC

noinst_LTLIBRARIES = libpaludislibxml.la

else

lib_LTLIBRARIES = libpaludislibxml.la

endif

paludis_libxml_includedir = $(includedir)/paludis/libxml
paludis_libxml_include_HEADERS = \
	libxml.hh
libpaludislibxml_la_SOURCES = $(paludis_libxml_include_HEADERS) \
	libxml.cc
libpaludislibxml_la_CXXFLAGS = $(AM_CXXFLAGS) @LIBXML2DEPS_CFLAGS@

if ! MONOLITHIC

libpaludislibxml_la_LIBADD = \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	@LIBXML2DEPS_LIBS@

endif

libpaludislibxml_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0

endif

EXTRA_DIST = libxml.hh libxml.cc

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

