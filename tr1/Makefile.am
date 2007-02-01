CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda memory
MAINTAINERCLEANFILES = Makefile.in

AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBDIR=\"$(libdir)\"

paludis_compat_tr1dir = $(libdir)/paludis/compat/tr1

if NEED_POINTER_WRAPPERS

paludis_compat_tr1_HEADERS = memory tr1_memory.hh

memory : tr1_memory.hh
	cp $(srcdir)/tr1_memory.hh $@

else

memory :
	rm -f $@ || true

endif

EXTRA_DIST = tr1_memory.hh
BUILT_SOURCES = memory
SUBDIRS = .

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done


