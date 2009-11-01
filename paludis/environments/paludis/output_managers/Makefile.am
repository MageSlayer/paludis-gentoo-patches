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

AUTOMAKE_OPTIONS = parallel-tests

shareenvpaludisdir = $(datarootdir)/paludis/environments/paludis/output_managers

dist_shareenvpaludis_DATA = \
	logged.conf \
	quiet.conf \
	standard.conf \
	standard_on_failure.conf \
	keep_messages.part.conf \
	keep_stdout_as_messages.part.conf \
	logged_only.part.conf \
	show_messages.part.conf

logdir = @HACKED_LOG_DIR@

log_DATA = .keep

.keep :
	touch $@

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

