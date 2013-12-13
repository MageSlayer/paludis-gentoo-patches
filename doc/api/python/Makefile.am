SUBDIRS = .

CLEANFILES = *~ python/*/* *.epicfail
MAINTAINERCLEANFILES = Makefile.in

examples = \
	example_about.py \
	example_command_line.py \
	example_version_spec.py

rst_docs = \
	example_about.rst \
	example_command_line.rst \
	example_version_spec.rst \
	index.rst \
	paludis.rst

EXTRA_DIST = conf.py \
	     $(rst_docs) \
	     $(examples)

if ENABLE_PYTHON_DOC

all-local : sphinx

sphinx : python

python : $(top_builddir)/python/paludis.so $(examples) $(rst_docs)
	mkdir -p $(builddir)/python && \
	env \
	    LD_LIBRARY_PATH="`echo $$LD_LIBRARY_PATH: | sed -e 's,^:,,'`` \
		    $(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/.libs/ \
		    `:`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/util/.libs/ \
		    `:`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/ruby/.libs/ \
		    `:`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/python/.libs/ \
		    `:`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories/e/.libs/`" \
	    PYTHONPATH="$(top_builddir)/python/:$(top_builddir)/python/.libs/" \
	    $(SPHINX_BUILD) -c $(builddir) -b html -d $(builddir)/doctrees $(srcdir) $(builddir)/python


install-data-local :
	mkdir -p $(DESTDIR)$(htmldir)/api
	cp -R python/ $(DESTDIR)$(htmldir)/api/

else

sphinx :
	@echo "You don't have python-doc turned on!"
	exit 1

endif


built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

htmldocs : htmldocs-subdirs

htmldocs-subdirs :
	for s in $(SUBDIRS) . ; do if test x$$s = x. ; then $(MAKE) htmldocs-local || exit 1 ; \
	    else $(MAKE) -C $$s htmldocs || exit 1 ; fi ; done

htmldocs-local : $(paludisapipythonhtml_DATA)

clean-local :
	rm -fr python/

uninstall-local :
	rm -fr $(DESTDIR)$(htmldir)/api/python

