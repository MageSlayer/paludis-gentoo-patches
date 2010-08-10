include $(top_srcdir)/misc/common-makefile.am

DISTCLEANFILES =
AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
EXTRA_DIST =
BUILT_SOURCES =

TESTS =

paludis_args_legacy_includedir = $(includedir)/paludis-$(PALUDIS_PC_SLOT)/paludis/args/legacy

paludis_args_legacy_include_HEADERS =

libpaludisargslegacy_@PALUDIS_PC_SLOT@_la_SOURCES =
libpaludisargslegacy_@PALUDIS_PC_SLOT@_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0

libpaludisargslegacy_@PALUDIS_PC_SLOT@_la_LIBADD = \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/args/libpaludisargs_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la

lib_LTLIBRARIES = libpaludisargslegacy_@PALUDIS_PC_SLOT@.la

check_PROGRAMS = $(TESTS)

test_ldadd = \
	$(top_builddir)/paludis/util/test_extras.o \
	libpaludisargslegacy_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/test/libtest.a \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la

