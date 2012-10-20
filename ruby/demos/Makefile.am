include $(top_srcdir)/misc/common-makefile.am

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

IF_RUBY_DEMOS = \
	version_spec_distributions.rb \
	latest_stable.rb \
	playman.rb

EXTRA_DIST = $(IF_RUBY_DEMOS)

if ENABLE_RUBY

rubydemosdir = $(datadir)/paludis/ruby/demos/
rubydemos_SCRIPTS = $(IF_RUBY_DEMOS)

install-exec-local :
	$(INSTALL) -d $(DESTDIR)$(bindir)
	ln -sf $(rubydemosdir)/playman.rb $(DESTDIR)$(bindir)/playman

uninstall-local :
	rm $(DESTDIR)$(bindir)/playman

endif

