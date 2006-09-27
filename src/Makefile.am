SUBDIRS = . paludis gtkpaludis qualudis adjutrix

CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda
MAINTAINERCLEANFILES = Makefile.in

noinst_LIBRARIES = libcolour.a
libcolour_a_SOURCES = colour.cc colour.hh
AM_CXXFLAGS = -I$(top_srcdir) -I$(top_srcdir)/src @PALUDIS_CXXFLAGS@

