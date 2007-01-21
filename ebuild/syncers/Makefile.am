MAINTAINERCLEANFILES = Makefile.in
CLEANFILES = *~ .keep docvs+ext docvs+pserver docvs+ssh dodarcs+http dodarcs+ssh dofile dogit dogit+http dogit+ssh dorsync dosvn dosvn+http dosvn+https dosvn+ssh
SUBDIRS = .

.keep :
	touch $@

libexecprogdir = $(libexecdir)/paludis/syncers/

libexecprog_SCRIPTS = \
	docvs+ext \
	docvs+pserver \
	docvs+ssh \
	dodarcs+http \
	dodarcs+ssh \
	dofile \
	dogit \
	dogit+http \
	dogit+ssh \
	dorsync \
	dosvn \
	dosvn+http \
	dosvn+https \
	dosvn+ssh

TESTS_ENVIRONMENT = env \
	PALUDIS_EBUILD_DIR="$(srcdir)/paludis/syncers/" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	bash $(top_srcdir)/test/run_test.sh

installusersyncersdir = $(datadir)/paludis/syncers
installusersyncers_DATA = .keep

TESTS =
EXTRA_DIST = $(libexecprog_SCRIPTS) docvs.in dodarcs.in dogit.in dorsync.in dosvn.in

check_SCRIPTS =
check_PROGRAMS = $(TESTS)

AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@

docvs+ext docvs+pserver docvs+ssh : docvs.in
	cat $? > $@
dodarcs+http dodarcs+ssh : dodarcs.in
	cat $? > $@
dogit dogit+http dogit+ssh : dogit.in
	cat $? > $@
dofile dorsync : dorsync.in
	cat $? > $@
dosvn dosvn+http dosvn+https dosvn+ssh : dosvn.in
	cat $? > $@

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

