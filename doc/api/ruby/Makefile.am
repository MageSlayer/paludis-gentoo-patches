SUBDIRS = .

CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda
MAINTAINERCLEANFILES = Makefile.in

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

distcheck-deps : $(DISTCHECK_DEPS) distcheck-deps-subdirs

distcheck-deps-subdirs :
	for s in `echo $(DIST_SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s distcheck-deps || exit 1 ; done

htmldocs : htmldocs-subdirs

htmldocs-subdirs :
	for s in $(SUBDIRS) . ; do if test x$$s = x. ; then $(MAKE) htmldocs-local || exit 1 ; \
	    else $(MAKE) -C $$s htmldocs || exit 1 ; fi ; done

htmldocs-local : $(paludisapirubyhtml_DATA)

dist_noinst_SCRIPTS = create_ruby_doc.rb

if ENABLE_RUBY

all-local : rdoc

rdoc : ruby

ruby : create_ruby_doc.rb
	rm -fr ruby
	$(RUBY) $(srcdir)/create_ruby_doc.rb -t "Paludis Ruby API" -m Paludis --op ruby $(top_srcdir)/ruby/*.cc

install-data-local :
	mkdir -p $(DESTDIR)$(htmldir)/api
	cp -R ruby/ $(DESTDIR)$(htmldir)/api/

endif

clean-local :
	rm -fr ruby

