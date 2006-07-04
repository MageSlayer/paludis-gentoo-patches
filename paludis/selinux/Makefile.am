MAINTAINERCLEANFILES = Makefile.in
CLEANFILES = *~
SUBDIRS = .

paludis_selinux_includedir = $(includedir)/paludis/selinux/

paludis_selinux_include_HEADERS = \
	security_context.hh

libpaludisselinux_a_SOURCES = \
	$(paludis_selinux_include_HEADERS) \
	security_context.cc

lib_LIBRARIES = libpaludisselinux.a

TESTS =
EXTRA_DIST =

check_PROGRAMS = $(TESTS)

AM_CXXFLAGS = -I$(top_srcdir)


