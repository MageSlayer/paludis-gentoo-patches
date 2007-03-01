SUBDIRS = . demos

AM_CXXFLAGS = -I$(top_srcdir) -I$(srcdir)/ \
	      -I$(top_builddir)/ \
	      @PALUDIS_CXXFLAGS_WITHOUT_PEDANTIC@ \
	      @PALUDIS_CXXFLAGS_NO_WREDUNDANT_DECLS@ \
	      @PALUDIS_CXXFLAGS_NO_WOLD_STYLE_CAST@

DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DLIBDIR=\"$(libdir)\"

CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda *.o Paludis.so .libs/Paludis.so
MAINTAINERCLEANFILES = Makefile.in

IF_RUBY_TESTS = \
	paludis_ruby_TEST.rb \
	log_TEST.rb \
	environment_TEST.rb \
	package_database_entry_TEST.rb \
	version_spec_TEST.rb \
	mask_reasons_TEST.rb \
	dep_spec_TEST.rb \
	repository_TEST.rb \
	version_metadata_TEST.rb \
	portage_dep_parser_TEST.rb \
	qualified_package_name_TEST.rb \
	contents_TEST.rb \
	dep_list_TEST.rb \
	dep_tag_TEST.rb \
	query_TEST.rb \
	package_database_TEST.rb

IF_RUBY_QA_TESTS = \
	message_TEST.rb \
	check_result_TEST.rb \
	metadata_file_TEST.rb \
	qa_environment_TEST.rb \
	check_TEST.rb

IF_RUBY_SOURCES = \
	paludis_ruby.cc paludis_ruby.hh \
	environment.cc \
	package_database_entry.cc \
	version_spec.cc \
	mask_reasons.cc \
	dep_spec.cc \
	log.cc \
	package_database.cc \
	repository.cc \
	version_metadata.cc \
	portage_dep_parser.cc \
	qualified_package_name.cc \
	contents.cc \
	dep_list.cc \
	dep_tag.cc \
	query.cc

IF_RUBY_QA_SOURCES = \
	message.cc \
	check.cc \
	check_result.cc \
	qa_environment.cc \
	metadata_file.cc

EXTRA_DIST = $(IF_RUBY_TESTS) $(IF_RUBY_SOURCES) \
	$(IF_RUBY_QA_TESTS) $(IF_RUBY_QA_SOURCES) \
	paludis_ruby_so.cc \
	environment_TEST_setup.sh environment_TEST_cleanup.sh \
	package_database_TEST_setup.sh package_database_TEST_cleanup.sh \
	repository_TEST_setup.sh repository_TEST_cleanup.sh \
	version_metadata_TEST_setup.sh version_metadata_TEST_cleanup.sh \
	paludis_ruby_TEST_setup.sh paludis_ruby_TEST_cleanup.sh \
	qa_environment_TEST_setup.sh qa_environment_TEST_cleanup.sh \
	check_TEST_setup.sh check_TEST_cleanup.sh \
	dep_list_TEST_setup.sh dep_list_TEST_cleanup.sh

TESTS_ENVIRONMENT = env \
	PALUDIS_NO_GLOBAL_HOOKS="yes" \
	PALUDIS_NO_XTERM_TITLES="yes" \
	PALUDIS_EBUILD_DIR="`$(top_srcdir)/paludis/repositories/gentoo/ebuild/utils/canonicalise $(top_srcdir)/paludis/repositories/gentoo/ebuild/`" \
	PALUDIS_EBUILD_DIR_FALLBACK="`$(top_srcdir)/paludis/repositories/gentoo/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories/gentoo/ebuild/`" \
	PALUDIS_REPOSITORY_SO_DIR="`$(top_srcdir)/paludis/repositories/gentoo/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories`" \
	PALUDIS_ENVIRONMENT_SO_DIR="`$(top_srcdir)/paludis/repositories/gentoo/ebuild/utils/canonicalise $(top_builddir)/paludis/environments`" \
	SYSCONFDIR="$(sysconfdir)" \
	LD_LIBRARY_PATH=`echo "\`$(top_srcdir)/paludis/repositories/gentoo/ebuild/utils/canonicalise $(top_builddir)/paludis/.libs\`: \
			\`$(top_srcdir)/paludis/repositories/gentoo/ebuild/utils/canonicalise $(top_builddir)/paludis/util/.libs\`: \
			\`$(top_srcdir)/paludis/repositories/gentoo/ebuild/utils/canonicalise $(top_builddir)/paludis/environments/.libs\`: \
			\`$(top_srcdir)/paludis/repositories/gentoo/ebuild/utils/canonicalise $(top_builddir)/paludis/environments/paludis/.libs\`: \
			\`$(top_srcdir)/paludis/repositories/gentoo/ebuild/utils/canonicalise $(top_builddir)/paludis/environments/no_config/.libs\`: \
			\`$(top_srcdir)/paludis/repositories/gentoo/ebuild/utils/canonicalise $(top_builddir)/paludis/dep_list/.libs\`: \
			\`$(top_srcdir)/paludis/repositories/gentoo/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories/.libs\`: \
			\`$(top_srcdir)/paludis/repositories/gentoo/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories/gentoo/.libs\`: \
			\`$(top_srcdir)/paludis/repositories/gentoo/ebuild/utils/canonicalise $(top_builddir)/paludis/qa/.libs\`: \
			\`$(top_srcdir)/paludis/repositories/gentoo/ebuild/utils/canonicalise $(top_builddir)/ruby/.libs\`" \
			| tr -d ' '` \
	bash $(top_srcdir)/test/run_test.sh "$(RUBY) -I ./.libs/"

if ENABLE_RUBY

lib_LTLIBRARIES = libpaludisruby.la

libpaludisruby_la_CXXFLAGS = $(AM_CXXFLAGS) -I. -I@RUBY_SITEARCHDIR@ -I@RUBY_ARCHDIR@

paludis_ruby_so.o : paludis_ruby_so.cc
	if $(LIBTOOL) --tag=CXX --mode=compile $(CXX) $(DEFS) $(DEFAULT_INCLUDES) \
	    $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(CXXFLAGS) $(AM_CXXFLAGS) -I$(RUBY_SITEARCHDIR) -I$(RUBY_ARCHDIR) -c \
	    -o $@ $(srcdir)/paludis_ruby_so.cc ; then ln -s .libs/paludis_ruby_so.o $@ ; else rm -f $@ ; exit 1 ; fi

if ENABLE_QA

libpaludisruby_la_SOURCES = $(IF_RUBY_SOURCES) $(IF_RUBY_QA_SOURCES)
libpaludisruby_la_LIBADD = \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/dep_list/libpaludisdeplist.la \
	$(top_builddir)/paludis/repositories/libpaludisrepositories.la \
	$(top_builddir)/paludis/repositories/gentoo/libpaludisgentoorepository.la \
	$(top_builddir)/paludis/environments/paludis/libpaludispaludisenvironment.la \
	$(top_builddir)/paludis/environments/no_config/libpaludisnoconfigenvironment.la \
	$(top_builddir)/paludis/environments/libpaludisenvironments.la \
	$(top_builddir)/paludis/qa/libpaludisqa.la

TESTS = $(IF_RUBY_QA_TESTS) $(IF_RUBY_TESTS)

else

libpaludisruby_la_SOURCES = $(IF_RUBY_SOURCES)
libpaludisruby_la_LIBADD = \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/dep_list/libpaludisdeplist.la \
	$(top_builddir)/paludis/repositories/libpaludisrepositories.la \
	$(top_builddir)/paludis/repositories/gentoo/libpaludisgentoorepository.la \
	$(top_builddir)/paludis/environments/paludis/libpaludispaludisenvironment.la \
	$(top_builddir)/paludis/environments/no_config/libpaludisnoconfigenvironment.la \
	$(top_builddir)/paludis/environments/libpaludisenvironments.la

TESTS = $(IF_RUBY_TESTS)

endif

check_DATA = .libs/Paludis.so
rubylibdir = @RUBY_INSTALL_DIR@
rubylib_DATA = Paludis.so

.libs/Paludis.so : libpaludisruby.la paludis_ruby_so.o
	mkdir -p .libs
	$(CXX) -fPIC -shared $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) \
	    $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) \
	    $(AM_LDFLAGS) $(LDFLAGS) \
	    -I@RUBY_SITEARCHDIR@ -l@RUBY_SONAME@ \
	    -o $@ \
	    .libs/paludis_ruby_so.o \
	    -L$(top_builddir)/ruby/.libs -lpaludisruby

Paludis.so : libpaludisruby.la paludis_ruby_so.o
	$(CXX) -fPIC -shared $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) \
	    $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) \
	    $(AM_LDFLAGS) $(LDFLAGS) \
	    -I@RUBY_SITEARCHDIR@ -l@RUBY_SONAME@ \
	    -o $@ \
	    .libs/paludis_ruby_so.o \
	    -L$(top_builddir)/ruby/.libs -lpaludisruby

endif

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done


