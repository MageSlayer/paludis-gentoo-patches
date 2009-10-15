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

CLEANFILES = *~ gmon.out *.gcov *.gcno  *.gcda *.loT *.o *.so *.epicfail
MAINTAINERCLEANFILES = Makefile.in

IF_RUBY_DEMOS = \
	version_spec_distributions.rb \
	find_unused_distfiles.rb \
	latest_stable.rb \
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
	PALUDIS_OPTIONS="" \
	PALUDIS_REPOSITORY_SO_DIR="`$(top_srcdir)/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories`" \
	PALUDIS_DEFAULT_OUTPUT_CONF="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_srcdir)/paludis/environments/paludis/tests_output.conf`" \
	PALUDIS_OUTPUT_MANAGERS_DIR="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_srcdir)/paludis/environments/paludis/output_managers/`" \
	SYSCONFDIR="$(sysconfdir)" \
	LD_LIBRARY_PATH="`$(top_srcdir)/ebuild/utils/canonicalise $(top_builddir)/paludis/.libs`" \
	bash $(top_srcdir)/test/run_test.sh "ruby -I ../.libs/"

if ENABLE_RUBY

rubydemosdir = $(datadir)/paludis/ruby/demos/
rubydemos_SCRIPTS = $(IF_RUBY_DEMOS)

install-exec-local :
	$(INSTALL) -d $(DESTDIR)$(bindir)
	ln -sf $(rubydemosdir)/playman.rb $(DESTDIR)$(bindir)/playman

uninstall-local :
	rm $(DESTDIR)$(bindir)/playman

endif

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

