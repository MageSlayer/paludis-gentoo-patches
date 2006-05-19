MAINTAINERCLEANFILES = Makefile.in Makefile.am
CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda .keep
SUBDIRS = .

installeselectdir = $(datadir)/eselect/modules/

installeselect_SCRIPTS = \
	news.eselect

TESTS_ENVIRONMENT = env \
	PALUDIS_EBUILD_DIR="$(srcdir)/ebuild/" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	bash $(top_srcdir)/ebuild/run_test.bash

TESTS =
EXTRA_DIST = \
	$(installeselect_SCRIPTS) \
	$(TESTS)

check_SCRIPTS = $(TESTS)
check_PROGRAMS =

