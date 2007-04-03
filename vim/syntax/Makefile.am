MAINTAINERCLEANFILES = Makefile.in
CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda .keep
SUBDIRS =

if HAVE_VIM

vimsyntaxdir = $(VIM_INSTALL_DIR)/syntax/

vimsyntax_DATA = \
	paludis-environment-conf.vim \
	paludis-keywords-conf.vim \
	paludis-licenses-conf.vim \
	paludis-mirrors-conf.vim \
	paludis-package-mask-conf.vim \
	paludis-repositories-conf.vim \
	paludis-use-conf.vim

endif

EXTRA_DIST = $(vimsyntax_DATA)

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done


