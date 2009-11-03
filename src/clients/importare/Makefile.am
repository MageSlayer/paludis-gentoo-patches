include $(top_srcdir)/misc/common-makefile.am

AM_CXXFLAGS = -I$(top_srcdir) -I$(top_srcdir)/src \
	      @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@

SUBDIRS = .

bin_PROGRAMS = importare
noinst_PROGRAMS = man-importare
man_MANS = importare.1

importare.1 : man-importare
	./man-importare > $@

man_importare_SOURCES = \
	man_importare.cc \
	command_line.hh \
	command_line.cc

man_importare_LDADD = \
	$(top_builddir)/paludis/args/libpaludisargs_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/args/libpaludisman_@PALUDIS_PC_SLOT@.a \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/libpaludismanpagethings_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/src/output/liboutput.a \
	$(DYNAMIC_LD_LIBS)

importare_SOURCES = \
	command_line.hh command_line.cc \
	install.hh install.cc \
	importare.cc

importare_LDADD = \
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

