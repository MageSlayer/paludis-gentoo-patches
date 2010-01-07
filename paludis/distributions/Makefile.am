MAINTAINERCLEANFILES = Makefile.in
CLEANFILES = *~ gmon.out *.gcov *.gcno  *.gcda *.loT .keep *.epicfail
SUBDIRS = . exherbo gentoo

AUTOMAKE_OPTIONS = 1.11 parallel-tests

distributions = \
	exherbo.conf \
	gentoo.conf

distributionsdir = $(datadir)/paludis/distributions/

distributions_DATA = $(distributions)

EXTRA_DIST = $(distributions)

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

