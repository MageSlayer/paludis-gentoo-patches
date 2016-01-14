include $(top_srcdir)/misc/common-makefile.am

SUBDIRS = .

paludis_selinux_includedir = $(includedir)/paludis-$(PALUDIS_PC_SLOT)/paludis/selinux/

paludis_selinux_include_HEADERS = \
	security_context.hh

libpaludisselinux_@PALUDIS_PC_SLOT@_la_SOURCES = \
	$(paludis_selinux_include_HEADERS) \
	security_context.cc

libpaludisselinux_@PALUDIS_PC_SLOT@_la_LIBADD = \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	@DYNAMIC_LD_LIBS@

libpaludisselinux_@PALUDIS_PC_SLOT@_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0

lib_LTLIBRARIES = libpaludisselinux_@PALUDIS_PC_SLOT@.la

TESTS =
EXTRA_DIST =

check_PROGRAMS = $(TESTS)

AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@

