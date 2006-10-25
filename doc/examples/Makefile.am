CLEANFILES = *~
MAINTAINERCLEANFILES = Makefile.in
AM_CXXFLAGS = -I$(top_srcdir) -I$(top_srcdir)/src @PALUDIS_CXXFLAGS@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DLIBDIR=\"$(libdir)\"

SUBDIRS = .
noinst_PROGRAMS = \
	pwp_basic_cplusplus_app

noinst_SCRIPTS = \
	pwp_basic_ruby_app.rb

EXTRA_DIST = $(check_PROGRAMS) $(check_SCRIPTS)

pwp_basic_cplusplus_app_SOURCES = pwp_basic_cplusplus_app.cc
pwp_basic_cplusplus_app_LDFLAGS = \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/environment/default/libpaludisdefaultenvironment.la

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

