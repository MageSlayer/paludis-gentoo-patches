SUBDIRS = .
CLEANFILES = *~ gmon.out *.gcov *.gcno  *.gcda *.loT *.epicfail .keep
BUILT_SOURCES =
MAINTAINERCLEANFILES = Makefile.in
AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBDIR=\"$(libdir)\"

shareenvpaludisdir = $(sharedir)/paludis/environments/paludis/output_managers

dist_shareenvpaludis_DATA = \
	keep_messages.conf \
	keep_stdout_as_messages.conf \
	logged.conf \
	logged_only.conf \
	quiet.conf \
	show_messages.conf \
	standard.conf

logdir = /var/log/paludis

log_DATA = .keep

.keep :
	touch $@

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

distcheck-deps-local : $(DISTCHECK_DEPS)

distcheck-deps : distcheck-deps-subdirs

distcheck-deps-subdirs :
	for s in $(SUBDIRS) . ; do if test x$$s = x. ; then $(MAKE) distcheck-deps-local || exit 1 ; \
	    else $(MAKE) -C $$s distcheck-deps || exit 1 ; fi ; done


