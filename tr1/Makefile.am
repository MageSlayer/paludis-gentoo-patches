CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda memory type_traits
MAINTAINERCLEANFILES = Makefile.in

AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBDIR=\"$(libdir)\"

paludis_compat_tr1dir = $(includedir)/paludis/compat/tr1

paludis_compat_tr1_HEADERS = tr1_memory.hh tr1_type_traits.hh

if NEED_POINTER_WRAPPERS
if NEED_TYPE_TRAITS_WRAPPERS
nodist_paludis_compat_tr1_HEADERS = memory type_traits
else
nodist_paludis_compat_tr1_HEADERS = memory
endif
else
if NEED_TYPE_TRAITS_WRAPPERS
nodist_paludis_compat_tr1_HEADERS = type_traits
else
nodist_paludis_compat_tr1_HEADERS =
endif
endif

if NEED_POINTER_WRAPPERS

memory : tr1_memory.hh
	cp $(srcdir)/tr1_memory.hh $@

else

memory :
	rm -f $@ || true

endif

if NEED_TYPE_TRAITS_WRAPPERS

type_traits : tr1_type_traits.hh
	cp $(srcdir)/tr1_type_traits.hh $@

else

type_traits :
	rm -f $@ || true

endif


EXTRA_DIST = tr1_memory.hh tr1_type_traits.hh
BUILT_SOURCES = memory type_traits
SUBDIRS = .

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done


