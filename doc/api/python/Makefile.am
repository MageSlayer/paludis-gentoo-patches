SUBDIRS = .

CLEANFILES = *~ python/*/* *.epicfail
MAINTAINERCLEANFILES = Makefile.in

examples = \
	example_about.py \
	example_command_line.py \
	example_version_spec.py

EXTRA_DIST = epydoc.css \
	     python_syntax.css \
	     $(examples)

dist_noinst_SCRIPTS = create_python_examples.py

if ENABLE_PYTHON_DOC

all-local : epydoc

epydoc : python

python : $(top_builddir)/python/paludis.so epydoc.css python_syntax.css create_python_examples.py $(examples)
	rm -fr python
	mkdir python
	env \
	    LD_LIBRARY_PATH=`echo "\`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/.libs\`: \
	    \`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/util/.libs\`: \
	    \`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/environments/paludis/.libs\`: \
	    \`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/environments/no_config/.libs\`: \
	    \`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories/e/.libs\`: \
	    \`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories/fake/.libs\`: \
	    \`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/python/.libs\`" \
	    | tr -d ' '` \
	    PYTHONPATH="./.libs" \
	    epydoc --name Paludis -o python --url http://paludis.pioto.org --no-frames --css $(srcdir)/epydoc.css \
	        $(top_builddir)/python/paludis.so
	$(PYTHON) $(srcdir)/create_python_examples.py $(srcdir) $(examples)
	find python/ -type f | xargs sed -i -e 's@http://###EX###@@'
	cp $(srcdir)/python_syntax.css python/


install-data-local :
	mkdir -p $(DESTDIR)$(htmldir)/api
	cp -R python/ $(DESTDIR)$(htmldir)/api/

else

epydoc :
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

