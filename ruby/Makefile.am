SUBDIRS = . demos

AM_CXXFLAGS = -I$(top_srcdir) -I$(srcdir)/ \
	      -I$(top_builddir)/ \
	      @PALUDIS_CXXFLAGS_WITHOUT_PEDANTIC@ \
	      @PALUDIS_CXXFLAGS_NO_WREDUNDANT_DECLS@ \
	      @PALUDIS_CXXFLAGS_NO_WOLD_STYLE_CAST@ \
	      @PALUDIS_CXXFLAGS_VISIBILITY@

DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DLIBDIR=\"$(libdir)\"

CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda *.o Paludis.so .libs/Paludis.so
MAINTAINERCLEANFILES = Makefile.in

IF_RUBY_TESTS = \
	action_TEST.rb \
	contents_TEST.rb \
	dep_list_TEST.rb \
	dep_spec_TEST.rb \
	dep_tag_TEST.rb \
	environment_TEST.rb \
	find_unused_packages_task_TEST.rb \
	log_TEST.rb \
	package_database_TEST.rb \
	package_id_TEST.rb \
	paludis_ruby_TEST.rb \
	qualified_package_name_TEST.rb \
	query_TEST.rb \
	repository_TEST.rb \
	version_spec_TEST.rb

IF_RUBY_QA_TESTS = \
	qa_TEST.rb

IF_RUBY_SOURCES = \
	paludis_ruby.cc paludis_ruby.hh \
	action.cc \
	contents.cc \
	dep_list.cc \
	dep_spec.cc \
	dep_tag.cc \
	environment.cc \
	find_unused_packages_task.cc \
	log.cc \
	metadata_key.cc \
	name.cc \
	package_database.cc \
	package_id.cc \
	qualified_package_name.cc \
	query.cc \
	repository.cc \
	version_spec.cc

IF_RUBY_QA_SOURCES = \
	qa.cc

EXTRA_DIST = $(IF_RUBY_TESTS) $(IF_RUBY_SOURCES) \
	$(IF_RUBY_QA_TESTS) $(IF_RUBY_QA_SOURCES) \
	paludis_ruby_so.cc \
	dep_list_TEST_setup.sh dep_list_TEST_cleanup.sh \
	environment_TEST_setup.sh environment_TEST_cleanup.sh \
	find_unused_packages_task_TEST_setup.sh find_unused_packages_task_TEST_cleanup.sh \
	package_database_TEST_setup.sh package_database_TEST_cleanup.sh \
	package_id_TEST_setup.sh package_id_TEST_cleanup.sh \
	paludis_ruby_TEST_setup.sh paludis_ruby_TEST_cleanup.sh \
	repository_TEST_setup.sh repository_TEST_cleanup.sh \
	action_TEST_setup.sh action_TEST_cleanup.sh

TESTS_ENVIRONMENT = env \
	TEST_OUTPUT_WRAPPER="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/util/outputwrapper`" \
	PALUDIS_OUTPUTWRAPPER_DIR="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/util/`" \
	PALUDIS_NO_GLOBAL_HOOKS="yes" \
	PALUDIS_NO_XTERM_TITLES="yes" \
	PALUDIS_EBUILD_DIR="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_srcdir)/paludis/repositories/e/ebuild/`" \
	PALUDIS_EBUILD_DIR_FALLBACK="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories/e/ebuild/`" \
	PALUDIS_EAPIS_DIR="$(top_srcdir)/paludis/repositories/e/eapis/" \
	PALUDIS_QA_DATA_DIR="$(top_srcdir)/paludis/repositories/e/qa/" \
	PALUDIS_DISTRIBUTIONS_DIR="$(top_srcdir)/paludis/distributions/" \
	PALUDIS_DISTRIBUTION="gentoo" \
	PALUDIS_REPOSITORY_SO_DIR="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories`" \
	PALUDIS_ENVIRONMENT_SO_DIR="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/environments`" \
	SYSCONFDIR="$(sysconfdir)" \
	LD_LIBRARY_PATH=`echo "\`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/.libs\`: \
			\`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/util/.libs\`: \
			\`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/environments/paludis/.libs\`: \
			\`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/environments/no_config/.libs\`: \
			\`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/environments/adapted/.libs\`: \
			\`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories/e/.libs\`: \
			\`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/qa/.libs\`: \
			\`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/ruby/.libs\`:$$LD_LIBRARY_PATH" \
			| tr -d ' '` \
	bash $(top_srcdir)/test/run_test.sh "$(RUBY) -I ./.libs/"

if ENABLE_RUBY

lib_LTLIBRARIES = libpaludisruby.la

libpaludisruby_la_CXXFLAGS = $(AM_CXXFLAGS) -I. -I@RUBY_SITEARCHDIR@ -I@RUBY_ARCHDIR@
libpaludisruby_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0

paludis_ruby_so.o : paludis_ruby_so.cc
	if $(LIBTOOL) --tag=CXX --mode=compile $(CXX) $(DEFS) $(DEFAULT_INCLUDES) \
	    $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(CXXFLAGS) $(AM_CXXFLAGS) -I$(RUBY_SITEARCHDIR) -I$(RUBY_ARCHDIR) -c \
	    -o $@ $(srcdir)/paludis_ruby_so.cc ; then ln -fs .libs/paludis_ruby_so.o $@ ; else rm -f $@ ; exit 1 ; fi

if ENABLE_QA

libpaludisruby_la_SOURCES = $(IF_RUBY_SOURCES) $(IF_RUBY_QA_SOURCES)
TESTS = $(IF_RUBY_TESTS) $(IF_RUBY_QA_TESTS)

else

libpaludisruby_la_SOURCES = $(IF_RUBY_SOURCES)
TESTS = $(IF_RUBY_TESTS)

endif

libpaludisruby_la_LIBADD = \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/environments/paludis/libpaludispaludisenvironment.la \
	$(top_builddir)/paludis/environments/no_config/libpaludisnoconfigenvironment.la \
	$(top_builddir)/paludis/environments/adapted/libpaludisadaptedenvironment.la \
	$(PTHREAD_LIBS)

check_DATA = .libs/Paludis.so
rubylibdir = @RUBY_INSTALL_DIR@
rubylib_DATA = Paludis.so

.libs/Paludis.so : libpaludisruby.la paludis_ruby_so.o
	mkdir -p .libs
	$(CXX) -fPIC -shared $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) \
	    $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) \
	    $(AM_LDFLAGS) $(LDFLAGS) $(PTHREAD_LIBS) \
	    -I@RUBY_SITEARCHDIR@ -l@RUBY_SONAME@ \
	    -o $@ \
	    .libs/paludis_ruby_so.o \
	    -L$(top_builddir)/ruby/.libs -lpaludisruby

Paludis.so : libpaludisruby.la paludis_ruby_so.o
	$(CXX) -fPIC -shared $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) \
	    $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) \
	    $(AM_LDFLAGS) $(LDFLAGS) $(PTHREAD_LIBS) \
	    -I@RUBY_SITEARCHDIR@ -l@RUBY_SONAME@ \
	    -o $@ \
	    .libs/paludis_ruby_so.o \
	    -L$(top_builddir)/ruby/.libs -lpaludisruby

endif

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

distcheck-deps-local : $(DISTCHECK_DEPS)

distcheck-deps : distcheck-deps-subdirs

distcheck-deps-subdirs :
	for s in $(SUBDIRS) . ; do if test x$$s = x. ; then $(MAKE) distcheck-deps-local || exit 1 ; \
	    else $(MAKE) -C $$s distcheck-deps || exit 1 ; fi ; done
