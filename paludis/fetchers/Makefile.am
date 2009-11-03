include $(top_srcdir)/misc/common-makefile.am

SUBDIRS = . demos

CLEANFILES += .keep

.keep :
	touch $@

libexecprogdir = $(libexecdir)/paludis/fetchers/

libexecprog_SCRIPTS = \
	dohttp \
	dohttps \
	doftp \
	dofile
installuserfetchersdir = $(datadir)/paludis/fetchers
installuserfetchers_DATA = .keep

TESTS =
EXTRA_DIST = $(libexecprog_SCRIPTS) dowget.in

check_SCRIPTS =
check_PROGRAMS = $(TESTS)

AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@

dohttp dohttps doftp : dowget.in
	cat $? > $@

