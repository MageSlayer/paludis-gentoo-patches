include $(top_srcdir)/misc/common-makefile.am

AM_CXXFLAGS = -I$(top_srcdir) -I$(top_srcdir)/src \
	      @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_NO_WOLD_STYLE_CAST@ @PALUDIS_CXXFLAGS_VISIBILITY@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DLIBDIR=\"$(libdir)\" \
	-DPYTHONINSTALLDIR=\"$(PYTHON_INSTALL_DIR)\" \
	-DRUBYINSTALLDIR=\"$(RUBY_INSTALL_DIR)\"

SUBDIRS = .

bin_PROGRAMS = paludis
noinst_PROGRAMS = man-paludis
man_MANS = paludis.1

paludis.1 : man-paludis
	./man-paludis > $@

man_paludis_SOURCES = \
	man_paludis.cc \
	command_line.hh \
	command_line.cc

man_paludis_LDADD = \
	$(top_builddir)/paludis/args/libpaludisargs_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/args/libpaludisman_@PALUDIS_PC_SLOT@.a \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/libpaludismanpagethings_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/src/output/liboutput.a \
	$(DYNAMIC_LD_LIBS)

paludis_SOURCES = \
	applets.hh           applets.cc           \
	command_line.hh      command_line.cc      \
	info.hh              info.cc              \
	install.hh           install.cc           \
	uninstall.hh         uninstall.cc         \
	list.hh              list.cc              \
	query.hh             query.cc             \
	sync.hh              sync.cc              \
	do_contents.hh       do_contents.cc       \
	do_executables.hh    do_executables.cc    \
	do_config.hh         do_config.cc         \
	owner.hh             owner.cc             \
	report.hh            report.cc            \
	paludis.cc

paludis_LDADD = \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/args/libpaludisargs_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/src/output/liboutput.a \
	$(DYNAMIC_LD_LIBS)

TESTS = \
	upgrade_TEST \
	install_TEST \
	continue_on_failure_TEST \
	version_TEST help_TEST \
	list_repository_formats_TEST \
	exception_TEST \
	pretend_TEST \
	args_from_environment_TEST

EXTRA_DIST = \
	$(man_MANS) \
	$(TESTS) \
	install_TEST_setup.sh install_TEST_cleanup.sh \
	pretend_TEST_setup.sh pretend_TEST_cleanup.sh \
	upgrade_TEST_setup.sh upgrade_TEST_cleanup.sh \
	continue_on_failure_TEST_setup.sh continue_on_failure_TEST_cleanup.sh

DISTCLEANFILES = $(man_MANS) $(noinst_DATA)

