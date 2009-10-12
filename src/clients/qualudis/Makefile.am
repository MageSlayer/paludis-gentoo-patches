AM_CXXFLAGS = -I$(top_srcdir) -I$(top_srcdir)/src @PALUDIS_CXXFLAGS@  @PALUDIS_CXXFLAGS_VISIBILITY@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DBIGTEMPDIR=\"/var/tmp\"
SUBDIRS = .

man_pages = qualudis.1

if ENABLE_QA

bin_PROGRAMS = qualudis
noinst_PROGRAMS = man-qualudis
man_MANS = $(man_pages)

qualudis_SOURCES = \
	qualudis_command_line.hh      qualudis_command_line.cc      \
	qualudis.cc

qualudis_LDADD = \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/args/libpaludisargs_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/selinux/libpaludisselinux_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/src/output/liboutput.a \
	$(PCRECPPDEPS_LIBS) \
	$(LIBXML2DEPS_LIBS) \
	$(DYNAMIC_LD_LIBS)

INCLUDES = $(PCRECPPDEPS_CFLAGS) $(LIBXML2DEPS_CFLAGS)

TESTS_ENVIRONMENT = env \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	PALUDIS_REPOSITORY_SO_DIR="$(top_builddir)/paludis/repositories" \
	PALUDIS_NO_XTERM_TITLES="yes" \
	PALUDIS_OPTIONS="" \
	QUALUDIS_OPTIONS="" \
	bash $(top_srcdir)/test/run_test.sh bash

TESTS = version_TEST

version_TEST :
	echo -e "#!/bin/sh\n./qualudis --version" > $@

endif

qualudis.1 : man-qualudis
	./man-qualudis > $@

man_qualudis_SOURCES = \
	man_qualudis.cc \
	qualudis_command_line.hh \
	qualudis_command_line.cc

man_qualudis_LDADD = \
	$(top_builddir)/paludis/args/libpaludisargs_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/args/libpaludisman_@PALUDIS_PC_SLOT@.a \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/libpaludismanpagethings_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/src/output/liboutput.a \
	$(DYNAMIC_LD_LIBS)

CLEANFILES = *~ version_TEST gmon.out *.gcov *.gcno  *.gcda *.loT *.epicfail
DISTCLEANFILES = $(man_pages) $(noinst_DATA)
MAINTAINERCLEANFILES = Makefile.in
EXTRA_DIST = $(man_pages)

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

DISTCHECK_DEPS = qualudis.1

distcheck-deps-local : $(DISTCHECK_DEPS)

distcheck-deps : distcheck-deps-subdirs

distcheck-deps-subdirs :
	for s in $(SUBDIRS) . ; do if test x$$s = x. ; then $(MAKE) distcheck-deps-local || exit 1 ; \
	    else $(MAKE) -C $$s distcheck-deps || exit 1 ; fi ; done

