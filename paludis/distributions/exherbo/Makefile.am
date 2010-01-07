MAINTAINERCLEANFILES = Makefile.in
CLEANFILES = *~ gmon.out *.gcov *.gcno  *.gcda *.loT .keep *.epicfail
SUBDIRS =

AUTOMAKE_OPTIONS = 1.11 parallel-tests

distributiondir = $(datadir)/paludis/distributions/exherbo/

distribution_DATA = e.conf gems.conf paludis.conf repository_blacklist.conf

EXTRA_DIST = $(distribution_DATA)

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

