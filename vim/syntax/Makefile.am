MAINTAINERCLEANFILES = Makefile.in
CLEANFILES = *~ gmon.out *.gcov *.gcno  *.gcda *.loT .keep *.epicfail
SUBDIRS =

vimscripts = \
	paludis-environment-conf.vim \
	paludis-keywords-conf.vim \
	paludis-licenses-conf.vim \
	paludis-mirrors-conf.vim \
	paludis-package-mask-conf.vim \
	paludis-repositories-conf.vim \
	paludis-use-conf.vim

if HAVE_VIM

vimsyntaxdir = $(VIM_INSTALL_DIR)/syntax/

vimsyntax_DATA = $(vimscripts)

endif

EXTRA_DIST = $(vimscripts)


built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

