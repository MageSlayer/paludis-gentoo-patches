MAINTAINERCLEANFILES = Makefile.in
CLEANFILES = *~ *.epicfail
SUBDIRS = .

AUTOMAKE_OPTIONS = parallel-tests

paludis_selinux_includedir = $(includedir)/paludis-$(PALUDIS_PC_SLOT)/paludis/selinux/

paludis_selinux_include_HEADERS = \
	security_context.hh

libpaludisselinux_@PALUDIS_PC_SLOT@_la_SOURCES = \
	$(paludis_selinux_include_HEADERS) \
	security_context.cc

if ! MONOLITHIC

libpaludisselinux_@PALUDIS_PC_SLOT@_la_LIBADD = \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	@DYNAMIC_LD_LIBS@

endif

libpaludisselinux_@PALUDIS_PC_SLOT@_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0

if MONOLITHIC

noinst_LTLIBRARIES = libpaludisselinux_@PALUDIS_PC_SLOT@.la

else

lib_LTLIBRARIES = libpaludisselinux_@PALUDIS_PC_SLOT@.la

endif

TESTS =
EXTRA_DIST =

check_PROGRAMS = $(TESTS)

AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

