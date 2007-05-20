MAINTAINERCLEANFILES = Makefile.in
CLEANFILES = *~
SUBDIRS = .

libexecprogdir = $(libexecdir)/paludis/digests/

paludis_digests_includedir = $(includedir)/paludis/digests/

paludis_digests_include_HEADERS = \
	sha256.hh \
	rmd160.hh \
	md5.hh

libpaludisdigests_la_SOURCES = \
	$(paludis_digests_include_HEADERS) \
	sha256.cc \
	rmd160.cc \
	md5.cc

libpaludisdigests_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0

if MONOLITHIC

noinst_LTLIBRARIES = libpaludisdigests.la

else

lib_LTLIBRARIES = libpaludisdigests.la

endif

TESTS = sha256_TEST rmd160_TEST md5_TEST
EXTRA_DIST = sha256_TEST.cc rmd160_TEST.cc md5_TEST.cc

check_PROGRAMS = $(TESTS)

AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@

sha256_TEST_SOURCES = sha256_TEST.cc
sha256_TEST_CXXFLAGS = $(AM_CXXFLAGS) -I$(top_srcdir)
sha256_TEST_LDADD = $(top_builddir)/test/libtest.a libpaludisdigests.la

rmd160_TEST_SOURCES = rmd160_TEST.cc
rmd160_TEST_CXXFLAGS = $(AM_CXXFLAGS) -I$(top_srcdir)
rmd160_TEST_LDADD = $(top_builddir)/test/libtest.a libpaludisdigests.la

md5_TEST_SOURCES = md5_TEST.cc
md5_TEST_CXXFLAGS = $(AM_CXXFLAGS) -I$(top_srcdir)
md5_TEST_LDADD = $(top_builddir)/test/libtest.a libpaludisdigests.la

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

