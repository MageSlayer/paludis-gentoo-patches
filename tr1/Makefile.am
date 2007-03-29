CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda memory type_traits
MAINTAINERCLEANFILES = Makefile.in

AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBDIR=\"$(libdir)\"

paludis_compat_tr1dir = $(includedir)/paludis/compat/tr1

paludis_compat_tr1_HEADERS = tr1_memory.hh tr1_type_traits.hh tr1_functional.hh

if NEED_POINTER_WRAPPERS

pointer_wrappers = memory

memory : tr1_memory.hh
	cp $(srcdir)/tr1_memory.hh $@

else

pointer_wrappers =

memory :
	rm -f $@ || true

endif

if NEED_TYPE_TRAITS_WRAPPERS
type_traits_wrappers = type_traits

type_traits : tr1_type_traits.hh
	cp $(srcdir)/tr1_type_traits.hh $@

else

type_traits_wrappers =

type_traits :
	rm -f $@ || true

endif

if NEED_BIND_WRAPPERS

bind_wrappers = functional

functional : tr1_functional.hh
	cp $(srcdir)/tr1_functional.hh $@

else

bind_wrappers =

functional :
	rm -f $@ || true

endif

nodist_paludis_compat_tr1_HEADERS = \
	$(pointer_wrappers) \
	$(type_traits_wrappers) \
	$(bind_wrappers)

EXTRA_DIST = tr1_memory.hh tr1_type_traits.hh tr1_functional.hh
BUILT_SOURCES = memory type_traits functional
SUBDIRS = .

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done


