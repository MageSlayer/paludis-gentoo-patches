CLEANFILES = *~
MAINTAINERCLEANFILES = Makefile.in svn-version-filter-data.bash
EXTRA_DIST = generated-file.txt svn-version-filter.bash svn-version-filter-data.bash.in \
	     do_m4.bash make_sr.bash make_se.bash

built-sources : $(BUILT_SOURCES)
	for s in $(SUBDIRS) ; do $(MAKE) -C $$s built-sources || exit 1 ; done

