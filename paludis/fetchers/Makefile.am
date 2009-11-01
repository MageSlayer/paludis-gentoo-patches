MAINTAINERCLEANFILES = Makefile.in
CLEANFILES = *~ dohttp dohttps doftp .keep *.epicfail
SUBDIRS = . demos

AUTOMAKE_OPTIONS = parallel-tests

.keep :
	touch $@

libexecprogdir = $(libexecdir)/paludis/fetchers/

libexecprog_SCRIPTS = \
	dohttp \
	dohttps \
	doftp \
	dofile

TESTS_ENVIRONMENT = env \
	PALUDIS_EBUILD_DIR="$(srcdir)/ebuild/" \
	PALUDIS_DISTRIBUTION="gentoo" \
	PALUDIS_OPTIONS="" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	bash $(top_srcdir)/test/run_test.sh

installuserfetchersdir = $(datadir)/paludis/fetchers
installuserfetchers_DATA = .keep

TESTS =
EXTRA_DIST = $(libexecprog_SCRIPTS) dowget.in

check_SCRIPTS =
check_PROGRAMS = $(TESTS)

AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@

dohttp dohttps doftp : dowget.in
	cat $? > $@

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

