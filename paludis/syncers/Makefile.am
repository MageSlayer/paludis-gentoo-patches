MAINTAINERCLEANFILES = Makefile.in
CLEANFILES = *~ .keep dobzr dobzr+aftp dobzr+file dobzr+ftp dobzr+http dobzr+https dobzr+sftp dobzr+ssh docvs+ext docvs+pserver docvs+ssh dodarcs+file dodarcs+http dodarcs+https dodarcs+ssh dofile dogit dogit+file dogit+http dogit+https dogit+rsync dogit+ssh dohg+file dohg+http dohg+https dohg+ssh dohg+static-http dorsync dorsync+ssh dosvn dosvn+file dosvn+http dosvn+https dosvn+ssh dotar+file dotar+ftp dotar+http dotar+https *.epicfail
SUBDIRS = .

.keep :
	touch $@

libexecprogdir = $(libexecdir)/paludis/syncers/

libexecprog_SCRIPTS = \
	dobzr \
	dobzr+aftp \
	dobzr+file \
	dobzr+ftp \
	dobzr+http \
	dobzr+https \
	dobzr+sftp \
	dobzr+ssh \
	docvs+ext \
	docvs+pserver \
	docvs+ssh \
	dodarcs+file \
	dodarcs+http \
	dodarcs+https \
	dodarcs+ssh \
	dofile \
	dogit \
	dogit+file \
	dogit+http \
	dogit+https \
	dogit+rsync \
	dogit+ssh \
	dohg+file \
	dohg+http \
	dohg+https \
	dohg+ssh \
	dohg+static-http \
	dorsync \
	dorsync+ssh \
	dosvn \
	dosvn+file \
	dosvn+http \
	dosvn+https \
	dosvn+ssh \
	dotar+file \
	dotar+ftp \
	dotar+http \
	dotar+https \
	dodummy

TESTS_ENVIRONMENT = env \
	PALUDIS_EBUILD_DIR="$(srcdir)/paludis/syncers/" \
	PALUDIS_DISTRIBUTION="gentoo" \
	PALUDIS_OPTIONS="" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	bash $(top_srcdir)/test/run_test.sh

installusersyncersdir = $(datadir)/paludis/syncers
installusersyncers_DATA = .keep

TESTS =
EXTRA_DIST = $(libexecprog_SCRIPTS) dobzr.in docvs.in dodarcs.in dogit.in dohg.in dorsync.in dosvn.in dotar.in

check_SCRIPTS =
check_PROGRAMS = $(TESTS)

AUTOMAKE_OPTIONS = parallel-tests

AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@

dobzr dobzr+aftp dobzr+file dobzr+ftp dobzr+http dobzr+https dobzr+sftp dobzr+ssh : dobzr.in
	cat $? > $@
docvs+ext docvs+pserver docvs+ssh : docvs.in
	cat $? > $@
dodarcs+file dodarcs+http dodarcs+https dodarcs+ssh : dodarcs.in
	cat $? > $@
dogit dogit+file dogit+http dogit+https dogit+rsync dogit+ssh : dogit.in
	cat $? > $@
dohg+file dohg+http dohg+https dohg+ssh dohg+static-http : dohg.in
	cat $? > $@
dofile dorsync dorsync+ssh : dorsync.in
	cat $? > $@
dosvn dosvn+file dosvn+http dosvn+https dosvn+ssh : dosvn.in
	cat $? > $@
dotar+file dotar+ftp dotar+http dotar+https : dotar.in
	cat $? > $@

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

