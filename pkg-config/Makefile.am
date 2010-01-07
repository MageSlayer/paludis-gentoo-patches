MAINTAINERCLEANFILES = Makefile.in
pkgconfigdir = $(libdir)/pkgconfig/
pkgconfig_DATA = paludis.pc
EXTRA_DIST = paludis.pc.in
CLEANFILES = paludis.pc *.epicfail

AUTOMAKE_OPTIONS = 1.11 parallel-tests

built-sources : $(BUILT_SOURCES)
	for s in $(SUBDIRS) ; do $(MAKE) -C $$s built-sources || exit 1 ; done

paludis.pc : paludis.pc.in Makefile
	sed \
	    -e "s,#PALUDIS_PC_PREFIX#,$(prefix)," \
	    -e "s,#PALUDIS_PC_LIBDIR#,$(libdir)," \
	    -e "s,#PALUDIS_PC_INCLUDEDIR#,$(includedir)/paludis-$(PALUDIS_PC_SLOT)," \
	    -e "s,#PALUDIS_PC_VERSION#,$(PALUDIS_PC_VERSION)," \
	    -e "s,#PALUDIS_PC_LIBS#,$(PALUDIS_PC_LIBS)," \
	    -e "s,#PALUDIS_PC_CFLAGS#,$(PALUDIS_PC_CFLAGS)," \
	    < $(srcdir)/paludis.pc.in > paludis.pc

