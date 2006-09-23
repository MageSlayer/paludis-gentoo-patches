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

CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda *.o *.so
MAINTAINERCLEANFILES = Makefile.in

IF_RUBY_TESTS = paludis_ruby_TEST.rb name_TEST.rb
IF_RUBY_SOURCES = paludis_ruby.cc paludis_ruby.hh name.cc
EXTRA_DIST = $(IF_RUBY_TESTS) $(IF_RUBY_SOURCES)

TESTS_ENVIRONMENT = env \
	PALUDIS_NO_GLOBAL_HOOKS="yes" \
	PALUDIS_NO_XTERM_TITLES="yes" \
	PALUDIS_EBUILD_DIR="`$(top_srcdir)/ebuild/utils/canonicalise $(top_srcdir)/ebuild/`" \
	PALUDIS_EBUILD_DIR_FALLBACK="`$(top_srcdir)/ebuild/utils/canonicalise $(top_builddir)/ebuild/`" \
	PALUDIS_REPOSITORY_SO_DIR="`$(top_srcdir)/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories`" \
	LD_LIBRARY_PATH="`$(top_srcdir)/ebuild/utils/canonicalise $(top_builddir)/paludis/.libs`" \
	ruby -I ./

if ENABLE_RUBY

TESTS = $(IF_RUBY_TESTS)
OUR_OBJECTS = name.o paludis_ruby.o
noinst_DATA = $(OUR_OBJECTS)
rubylibdir = $(DESTDIR)/@RUBY_SITEARCHDIR@
rubylib_DATA = Paludis.so

OUR_CXXCOMPILE = $(CXX) -fPIC $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) \
	    $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) \
	    -I. -I@RUBY_SITEARCHDIR@ -I@RUBY_ARCHDIR@ -c

paludis_ruby.o : paludis_ruby.cc paludis_ruby.hh
	$(OUR_CXXCOMPILE) -o $@ $(srcdir)/paludis_ruby.cc

name.o : name.cc paludis_ruby.hh
	$(OUR_CXXCOMPILE) -o $@ $(srcdir)/name.cc

Paludis.so : $(OUR_OBJECTS)
	$(CXX) -fPIC -shared $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) \
	    $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) \
	    $(AM_LDFLAGS) $(LDFLAGS) -I@RUBY_SITEARCHDIR@ -l@RUBY_SONAME@ \
	    -L$(top_builddir)/paludis/.libs -lpaludis -o $@ $^

clean-local :
	rm -fr Paludis.so .checkimage || true

endif

