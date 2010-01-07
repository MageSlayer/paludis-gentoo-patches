MAINTAINERCLEANFILES = Makefile.in
CLEANFILES = *~ gmon.out *.gcov *.gcno  *.gcda *.loT .keep *.epicfail
SUBDIRS =

AUTOMAKE_OPTIONS = 1.11 parallel-tests

vimscripts = \
	paludis.vim

if HAVE_VIM

vimftdetectdir = $(VIM_INSTALL_DIR)/ftdetect/

vimftdetect_DATA = $(vimscripts)

endif

EXTRA_DIST = $(vimscripts)

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

