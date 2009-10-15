SUBDIRS = .

CLEANFILES = *~ gmon.out *.gcov *.gcno  *.gcda *.loT *.epicfail
MAINTAINERCLEANFILES = Makefile.in

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

htmldocs : htmldocs-subdirs

htmldocs-subdirs :
	for s in $(SUBDIRS) . ; do if test x$$s = x. ; then $(MAKE) htmldocs-local || exit 1 ; \
	    else $(MAKE) -C $$s htmldocs || exit 1 ; fi ; done

htmldocs-local : $(paludisapirubyhtml_DATA)

dist_noinst_SCRIPTS = create_ruby_doc.rb \
		      create_ruby_examples.rb

examples = \
	example_about.rb \
	example_action.rb \
	example_command_line.rb \
	example_contents.rb \
	example_dep_tree.rb \
	example_environment.rb \
	example_package_database.rb \
	example_package_id.rb \
	example_mask.rb \
	example_match_package.rb \
	example_repository.rb \
	example_selection.rb \
	example_version_operator.rb \
	example_version_spec.rb

EXTRA_DIST = $(examples) \
	     ruby_syntax.css

if ENABLE_RUBY_DOC

all-local : rdoc

rdoc : ruby

ruby : create_ruby_doc.rb create_ruby_examples.rb ruby_syntax.css $(examples)
	rm -fr ruby
	env TOP_SRCDIR="$(top_srcdir)" $(RUBY) \
	    $(srcdir)/create_ruby_doc.rb -t "Paludis Ruby API" -m Paludis --op ruby \
		$(top_srcdir)/ruby/*.cc
	$(RUBY) $(srcdir)/create_ruby_examples.rb $(srcdir) $(examples)
	cp $(srcdir)/ruby_syntax.css ruby

install-data-local :
	mkdir -p $(DESTDIR)$(htmldir)/api
	cp -R ruby/ $(DESTDIR)$(htmldir)/api/

endif

clean-local :
	rm -fr ruby

uninstall-local :
	rm -fr $(DESTDIR)$(htmldir)/api/ruby

