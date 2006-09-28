MAINTAINERCLEANFILES = Makefile.in
CLEANFILES = *~
SUBDIRS = .

paludis_selinux_includedir = $(includedir)/paludis/selinux/

paludis_selinux_include_HEADERS = \
	security_context.hh

libpaludisselinux_la_SOURCES = \
	$(paludis_selinux_include_HEADERS) \
	security_context.cc

libpaludisselinux_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0

lib_LTLIBRARIES = libpaludisselinux.la

TESTS =
EXTRA_DIST =

check_PROGRAMS = $(TESTS)

AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@


