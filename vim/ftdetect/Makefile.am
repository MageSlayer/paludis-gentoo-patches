MAINTAINERCLEANFILES = Makefile.in
CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda .keep
SUBDIRS =

if HAVE_VIM

vimftdetectdir = $(VIM_INSTALL_DIR)/ftdetect/

vimftdetect_DATA = \
	paludis.vim

endif

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done


