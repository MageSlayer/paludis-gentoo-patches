include $(top_srcdir)/misc/common-makefile.am

AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_NO_WOLD_STYLE_CAST@ @PALUDIS_CXXFLAGS_NO_WSHADOW@

SUBDIRS = .

noinst_LIBRARIES = libbrokenlinkagefinder.a

libbrokenlinkagefinder_a_SOURCES = \
	broken_linkage_finder.cc   broken_linkage_finder.hh   \
	configuration.cc           configuration.hh           \
	linkage_checker.cc         linkage_checker.hh         \
	libtool_linkage_checker.cc libtool_linkage_checker.hh \
	elf_linkage_checker.cc     elf_linkage_checker.hh

TESTS = configuration_TEST
check_PROGRAMS = $(TESTS)

EXTRA_DIST = \
	configuration_TEST_setup.sh configuration_TEST_cleanup.sh

configuration_TEST_SOURCES = configuration_TEST.cc
configuration_TEST_LDADD = \
	libbrokenlinkagefinder.a \
	../util/libreconcilioutil.a \
	$(top_builddir)/src/clients/reconcilio/littlelf/liblittlelf.a \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/test/libtest.a
configuration_TEST_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

