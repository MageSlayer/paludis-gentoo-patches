include $(top_srcdir)/misc/common-makefile.am

SUBDIRS = cran .
AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBDIR=\"$(libdir)\"

paludis_repositories_libdir = $(libdir)/paludis/repositories

noinst_LTLIBRARIES = libpaludiscranrepository.la

noinst_HEADERS = \
	cran_package_id.hh \
	cran_dep_parser.hh \
	cran_repository.hh \
	cran_installed_repository.hh \
	description_file.hh \
	masks.hh \
	keys.hh \
	normalise.hh \
	dep_spec_pretty_printer.hh \
	spec_tree_pretty_printer.hh \
	package_dep_spec.hh

libpaludiscranrepository_la_SOURCES = \
	cran_package_id.cc \
	cran_dep_parser.cc \
	cran_repository.cc \
	cran_installed_repository.cc \
	description_file.cc \
	registration.cc \
	normalise.cc \
	masks.cc \
	keys.cc \
	dep_spec_pretty_printer.cc \
	spec_tree_pretty_printer.cc \
	package_dep_spec.cc \
	$(paludis_repositories_cran_include_HEADERS)

TESTS = \
	cran_dep_parser_TEST \
	cran_repository_TEST \
	cran_installed_repository_TEST \
	description_file_TEST

cran_dep_parser_TEST_SOURCES = cran_dep_parser_TEST.cc

cran_dep_parser_TEST_LDADD = \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/test/libtest.a

cran_dep_parser_TEST_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

cran_installed_repository_TEST_SOURCES = cran_installed_repository_TEST.cc

cran_installed_repository_TEST_LDADD = \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/test/libtest.a \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(DYNAMIC_LD_LIBS)

cran_installed_repository_TEST_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

cran_repository_TEST_SOURCES = cran_repository_TEST.cc

cran_repository_TEST_LDADD = \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/test/libtest.a \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(DYNAMIC_LD_LIBS)

cran_repository_TEST_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

description_file_TEST_SOURCES = description_file_TEST.cc

description_file_TEST_LDADD = \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/test/libtest.a \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(DYNAMIC_LD_LIBS)

description_file_TEST_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

EXTRA_DIST = \
	cran_repository_TEST.cc \
	cran_repository_TEST_setup.sh \
	cran_repository_TEST_cleanup.sh \
	cran_installed_repository_TEST.cc \
	cran_installed_repository_TEST_setup.sh \
	cran_installed_repository_TEST_cleanup.sh

check_PROGRAMS = $(TESTS)
check_SCRIPTS = \
	cran_repository_TEST_setup.sh cran_repository_TEST_cleanup.sh \
	cran_installed_repository_TEST_setup.sh cran_installed_repository_TEST_cleanup.sh

