MAINTAINERCLEANFILES = Makefile.in
CLEANFILES = *~ dohttp dohttps doftp
SUBDIRS = .

libexecprogdir = $(libexecdir)/paludis/fetchers/

libexecprog_SCRIPTS = \
	dohttp \
	dohttps \
	doftp \
	dofile

TESTS_ENVIRONMENT = env \
	PALUDIS_EBUILD_DIR="$(srcdir)/ebuild/" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	bash $(top_srcdir)/test/run_test.sh

TESTS =
EXTRA_DIST = $(libexecprog_SCRIPTS) dowget.in

check_SCRIPTS =
check_PROGRAMS = $(TESTS)

AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@

dohttp dohttps doftp : dowget.in
	cat $< > $@

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

