include $(top_srcdir)/misc/common-makefile.am

AM_CXXFLAGS = -I$(top_srcdir) -I$(top_srcdir)/src \
	      @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@

SUBDIRS = .

bin_PROGRAMS = reconcilio
noinst_PROGRAMS = man-reconcilio
man_MANS = reconcilio.1

reconcilio.1 : man-reconcilio
	./man-reconcilio > $@

man_reconcilio_SOURCES = \
	man_reconcilio.cc \
	command_line.hh \
	command_line.cc

man_reconcilio_LDADD = \
	$(top_builddir)/paludis/args/libpaludisargs_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/args/libpaludisman_@PALUDIS_PC_SLOT@.a \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/libpaludismanpagethings_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/src/output/liboutput.a \
	$(DYNAMIC_LD_LIBS)

reconcilio_SOURCES = \
	command_line.hh	command_line.cc \
	fix_linkage.hh	fix_linkage.cc \
	install.hh	install.cc \
	reconcilio.cc

reconcilio_LDADD = \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/args/libpaludisargs_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/src/output/liboutput.a \
	$(DYNAMIC_LD_LIBS)

TESTS =

EXTRA_DIST = \
	$(man_MANS) \
	$(TESTS)

DISTCLEANFILES = $(man_MANS) $(noinst_DATA)

