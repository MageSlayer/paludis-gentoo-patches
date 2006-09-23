SUBDIRS = .

AM_CXXFLAGS = -I$(top_srcdir) -I$(srcdir)/ \
	      @PALUDIS_CXXFLAGS_WITHOUT_PEDANTIC@ \
	      @PALUDIS_CXXFLAGS_NO_WREDUNDANT_DECLS@ \
	      @PALUDIS_CXXFLAGS_NO_WOLD_STYLE_CAST@

DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DLIBDIR=\"$(libdir)\"

CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda *.o *.so .libs/*.so
MAINTAINERCLEANFILES = Makefile.in

IF_RUBY_TESTS = \
	paludis_ruby_TEST.rb \
	name_TEST.rb \
	default_environment_TEST.rb \
	package_database_entry_TEST.rb \
	version_spec_TEST.rb \
	mask_reasons_TEST.rb \
	log_TEST.rb

IF_RUBY_SOURCES = \
	paludis_ruby.cc paludis_ruby.hh \
	name.cc \
	default_environment.cc \
	package_database_entry.cc \
	version_spec.cc \
	mask_reasons.cc \
	log.cc

EXTRA_DIST = $(IF_RUBY_TESTS) $(IF_RUBY_SOURCES) \
	default_environment_TEST_setup.sh default_environment_TEST_cleanup.sh

TESTS_ENVIRONMENT = env \
	PALUDIS_NO_GLOBAL_HOOKS="yes" \
	PALUDIS_NO_XTERM_TITLES="yes" \
	PALUDIS_EBUILD_DIR="`$(top_srcdir)/ebuild/utils/canonicalise $(top_srcdir)/ebuild/`" \
	PALUDIS_EBUILD_DIR_FALLBACK="`$(top_srcdir)/ebuild/utils/canonicalise $(top_builddir)/ebuild/`" \
	PALUDIS_REPOSITORY_SO_DIR="`$(top_srcdir)/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories`" \
	SYSCONFDIR="$(sysconfdir)" \
	LD_LIBRARY_PATH="`$(top_srcdir)/ebuild/utils/canonicalise $(top_builddir)/paludis/.libs`:`$(top_srcdir)/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories/nothing/.libs`:`$(top_srcdir)/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories/virtuals/.libs`" \
	bash $(top_srcdir)/test/run_test.sh "ruby -I ./.libs/"

if ENABLE_RUBY

OUR_OBJECTS = \
	default_environment.o \
	log.o \
	mask_reasons.o \
	name.o \
	package_database_entry.o \
	version_spec.o \
	paludis_ruby.o

TESTS = $(IF_RUBY_TESTS)
noinst_DATA = $(OUR_OBJECTS)
check_DATA = .libs/Paludis.so
rubylibdir = $(DESTDIR)/@RUBY_INSTALL_DIR@
rubylib_DATA = Paludis.so

OUR_CXXCOMPILE = $(CXX) -fPIC $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) \
	    $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) \
	    -I. -I@RUBY_SITEARCHDIR@ -I@RUBY_ARCHDIR@ -c

paludis_ruby.o : paludis_ruby.cc paludis_ruby.hh
	$(OUR_CXXCOMPILE) -o $@ $(srcdir)/paludis_ruby.cc

name.o : name.cc paludis_ruby.hh
	$(OUR_CXXCOMPILE) -o $@ $(srcdir)/name.cc

default_environment.o : default_environment.cc paludis_ruby.hh
	$(OUR_CXXCOMPILE) -o $@ $(srcdir)/default_environment.cc

package_database_entry.o : package_database_entry.cc paludis_ruby.hh
	$(OUR_CXXCOMPILE) -o $@ $(srcdir)/package_database_entry.cc

version_spec.o : version_spec.cc paludis_ruby.hh
	$(OUR_CXXCOMPILE) -o $@ $(srcdir)/version_spec.cc

mask_reasons.o : mask_reasons.cc paludis_ruby.hh
	$(OUR_CXXCOMPILE) -o $@ $(srcdir)/mask_reasons.cc

log.o : log.cc paludis_ruby.hh
	$(OUR_CXXCOMPILE) -o $@ $(srcdir)/log.cc

.libs/Paludis.so : $(OUR_OBJECTS)
	mkdir -p .libs
	$(CXX) -fPIC -shared $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) \
	    $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) \
	    $(AM_LDFLAGS) $(LDFLAGS) \
	    -I@RUBY_SITEARCHDIR@ -l@RUBY_SONAME@ \
	    -o $@ \
	    $(top_builddir)/paludis/.libs/libpaludis.so \
	    $(top_builddir)/paludis/repositories/nothing/.libs/libpaludisnothingrepository.so \
	    $(top_builddir)/paludis/repositories/virtuals/.libs/libpaludisvirtualsrepository.so \
	    $^

Paludis.so : $(OUR_OBJECTS)
	mkdir -p .libs
	$(CXX) -fPIC -shared $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) \
	    $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) \
	    $(AM_LDFLAGS) $(LDFLAGS) \
	    -I@RUBY_SITEARCHDIR@ -l@RUBY_SONAME@ \
	    -L$(top_builddir)/paludis/util/.libs/ -lpaludisutil \
	    -L$(top_builddir)/paludis/.libs/ -lpaludis \
	    -o $@ \
	    $^


endif

