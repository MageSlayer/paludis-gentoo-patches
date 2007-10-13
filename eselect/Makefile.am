MAINTAINERCLEANFILES = Makefile.in
CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda .keep
SUBDIRS = .

installeselectdir = $(datadir)/eselect/modules/

installeselect_SCRIPTS = \
	repo.eselect

TESTS_ENVIRONMENT = env \
	PALUDIS_EBUILD_DIR="$(srcdir)/ebuild/" \
	PALUDIS_DISTRIBUTION="gentoo" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	bash $(top_srcdir)/ebuild/run_test.bash

TESTS =
EXTRA_DIST = \
	$(installeselect_SCRIPTS) \
	$(TESTS)

check_SCRIPTS = $(TESTS)
check_PROGRAMS =

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

distcheck-deps-local : $(DISTCHECK_DEPS)

distcheck-deps : distcheck-deps-subdirs

distcheck-deps-subdirs :
	for s in $(SUBDIRS) . ; do if test x$$s = x. ; then make distcheck-deps-local || exit 1 ; \
	    else $(MAKE) -C $$s distcheck-deps || exit 1 ; fi ; done

