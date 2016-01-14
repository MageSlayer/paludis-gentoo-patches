include $(top_srcdir)/misc/common-makefile.am

SUBDIRS = . $(BUILD_REPOSITORIES)

DIST_SUBDIRS = $(ALL_REPOSITORIES)

AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBDIR=\"$(libdir)\"

