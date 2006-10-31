AM_CXXFLAGS = -I$(top_srcdir) \
	@PALUDIS_CXXFLAGS@ \
	@PALUDIS_CXXFLAGS_NO_WOLD_STYLE_CAST@ \
	@PALUDIS_CXXFLAGS_NO_WREDUNDANT_DECLS@ \
	@PALUDIS_CXXFLAGS_NO_WSHADOW@

DEFS = \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DGLIBMM_EXCEPTIONS_ENABLED=1

SUBDIRS = .

INCLUDES = $(GTKDEPS_CFLAGS)

if ENABLE_GTK

noinst_LIBRARIES = libcellrendererbutton.a

libcellrendererbutton_a_SOURCES = \
	cellrendererbutton.cc cellrendererbutton.hh

endif

CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda
MAINTAINERCLEANFILES = Makefile.in

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done



