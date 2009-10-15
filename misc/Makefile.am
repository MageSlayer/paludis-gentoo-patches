CLEANFILES = *~ *.epicfail
MAINTAINERCLEANFILES = Makefile.in
EXTRA_DIST = generated-file.txt do_m4.bash make_se.bash make_nn.bash

built-sources : $(BUILT_SOURCES)
	for s in $(SUBDIRS) ; do $(MAKE) -C $$s built-sources || exit 1 ; done

