SUBDIRS = .

AM_CXXFLAGS = -I$(top_srcdir) -I$(srcdir)/ -I$(top_srcdir)/ruby/ \
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

IF_RUBY_DEMOS = \
	version_spec_distributions.rb \
	find_unused_distfiles.rb \
	latest_stable.rb \
	qualudis.rb \
	find_unmanaged_files.rb \
	playman.rb

EXTRA_DIST = $(IF_RUBY_DEMOS)

TESTS_ENVIRONMENT = env \
	PALUDIS_NO_GLOBAL_HOOKS="yes" \
	PALUDIS_NO_XTERM_TITLES="yes" \
	PALUDIS_EBUILD_DIR="`$(top_srcdir)/ebuild/utils/canonicalise $(top_srcdir)/ebuild/`" \
	PALUDIS_EBUILD_DIR_FALLBACK="`$(top_srcdir)/ebuild/utils/canonicalise $(top_builddir)/ebuild/`" \
	PALUDIS_EAPIS_DIR="$(top_srcdir)/paludis/eapis/" \
	PALUDIS_DISTRIBUTIONS_DIR="$(top_srcdir)/paludis/distributions/" \
	PALUDIS_DISTRIBUTION="gentoo" \
	PALUDIS_REPOSITORY_SO_DIR="`$(top_srcdir)/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories`" \
	SYSCONFDIR="$(sysconfdir)" \
	LD_LIBRARY_PATH="`$(top_srcdir)/ebuild/utils/canonicalise $(top_builddir)/paludis/.libs`" \
	bash $(top_srcdir)/test/run_test.sh "ruby -I ../.libs/"

if ENABLE_RUBY

rubydemosdir = $(datadir)/paludis/ruby/demos/
rubydemos_DATA = $(IF_RUBY_DEMOS)

endif

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

distcheck-deps-local : $(DISTCHECK_DEPS)

distcheck-deps : distcheck-deps-subdirs

distcheck-deps-subdirs :
	for s in $(SUBDIRS) . ; do if test x$$s = x. ; then $(MAKE) distcheck-deps-local || exit 1 ; \
	    else $(MAKE) -C $$s distcheck-deps || exit 1 ; fi ; done
