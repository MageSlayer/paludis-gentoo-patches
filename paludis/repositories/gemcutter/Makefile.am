include $(top_srcdir)/misc/common-makefile.am

SUBDIRS = .

AM_CXXFLAGS = -I$(top_srcdir) -I$(top_builddir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBDIR=\"$(libdir)\"

noinst_LTLIBRARIES = libpaludisgemcutterrepository.la

noinst_HEADERS = \
	gemcutter_dependencies_key.hh \
	gemcutter_id.hh gemcutter_id-fwd.hh \
	gemcutter_repository.hh gemcutter_repository-fwd.hh \
	gemcutter_repository_store.hh \
	gemcutter_uri_key.hh \
	json_common.hh json_common-fwd.hh \
	json_things_handle.hh json_things_handle-fwd.hh

libpaludisgemcutterrepository_la_SOURCES = \
	gemcutter_dependencies_key.cc \
	gemcutter_id.cc \
	gemcutter_uri_key.cc \
	gemcutter_repository.cc \
	gemcutter_repository_store.cc \
	json_common.cc \
	json_things_handle.cc \
	registration.cc \
	$(noinst_HEADERS)

lib_LTLIBRARIES = libpaludisgemcutterrepositoryjsonthings_@PALUDIS_PC_SLOT@.la

libpaludisgemcutterrepositoryjsonthings_@PALUDIS_PC_SLOT@_la_SOURCES = json_things.cc json_things.hh
libpaludisgemcutterrepositoryjsonthings_@PALUDIS_PC_SLOT@_la_CXXFLAGS = $(AM_CXXFLAGS) @JANSSONDEPS_CFLAGS@
libpaludisgemcutterrepositoryjsonthings_@PALUDIS_PC_SLOT@_la_LIBADD = @JANSSONDEPS_LIBS@
libpaludisgemcutterrepositoryjsonthings_@PALUDIS_PC_SLOT@_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0

