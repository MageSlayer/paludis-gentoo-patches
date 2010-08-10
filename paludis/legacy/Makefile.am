include $(top_srcdir)/misc/common-makefile.am

DISTCLEANFILES =
AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
EXTRA_DIST =
BUILT_SOURCES =

TESTS =

paludis_legacy_includedir = $(includedir)/paludis-$(PALUDIS_PC_SLOT)/paludis/legacy

paludis_legacy_include_HEADERS =

libpaludislegacy_@PALUDIS_PC_SLOT@_la_SOURCES =
libpaludislegacy_@PALUDIS_PC_SLOT@_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0

libpaludislegacy_@PALUDIS_PC_SLOT@_la_LIBADD = \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la

lib_LTLIBRARIES = libpaludislegacy_@PALUDIS_PC_SLOT@.la

check_PROGRAMS = $(TESTS)

test_ldadd = \
	$(top_builddir)/paludis/util/test_extras.o \
	libpaludislegacy_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/test/libtest.a \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la

