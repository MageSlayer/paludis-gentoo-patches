MAINTAINERCLEANFILES = Makefile.in
CLEANFILES = *~

libexecprogdir = $(libexecdir)/paludis/utils

libexecprog_SCRIPTS = \
	dobin \
	doconfd \
	dodir \
	dodoc \
	doenvd \
	doexe \
	dohard \
	dohtml \
	doinfo \
	doinitd \
	doins \
	dolib \
	dolib.a \
	dolib.so \
	doman \
	domo \
	donewins \
	dosbin \
	dosed \
	dosym \
	fowners \
	fperms \
	merge \
	newbin \
	newconfd \
	newdoc \
	newenvd \
	newexe \
	newinitd \
	newins \
	newlib.a \
	newlib.so \
	newman \
	newsbin \
	unmerge

TESTS_ENVIRONMENT = env \
	PALUDIS_EBUILD_DIR="$(top_srcdir)/ebuild/" \
	PALUDIS_EBUILD_LOG_LEVEL="silent" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	$(SHELL) $(top_srcdir)/ebuild/utils/run_test.bash

TESTS = merge_TEST.bash dobin_TEST.bash unmerge_TEST.bash
EXTRA_DIST = $(libexecprog_SCRIPTS) $(TESTS) run_test.bash

check_SCRIPTS = $(TESTS) \
	merge_TEST_setup.sh merge_TEST_cleanup.sh \
	unmerge_TEST_setup.sh unmerge_TEST_cleanup.sh

check_PROGRAMS =

