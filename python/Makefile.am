SUBDIRS = .

AM_CXXFLAGS = -I$(top_srcdir) -I$(srcdir)/ \
	      -I$(top_builddir)/ \
	      @PALUDIS_CXXFLAGS_WITHOUT_PEDANTIC@ \
	      @PALUDIS_CXXFLAGS_NO_WREDUNDANT_DECLS@ \
	      @PALUDIS_CXXFLAGS_NO_WUNUSED@ \
	      @PALUDIS_CXXFLAGS_NO_WSHADOW@ \
	      @PALUDIS_CXXFLAGS_NO_WEXTRA@ \
	      @PALUDIS_CXXFLAGS_NO_WOLD_STYLE_CAST@ \
	      @PALUDIS_CXXFLAGS_NO_WWRITE_STRINGS@ \
	      @PALUDIS_CXXFLAGS_VISIBILITY@

DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DLIBDIR=\"$(libdir)\"

CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda *.o paludis.so .libs/paludis.so
MAINTAINERCLEANFILES = Makefile.in

IF_PYTHON_TESTS = \
	version_spec_TEST.py \
	version_operator_TEST.py \
	version_requirements_TEST.py \
	mask_reasons_TEST.py \
	contents_TEST.py \
	dep_spec_TEST.py \
	portage_dep_parser_TEST.py \
	log_TEST.py \
	name_TEST.py \
	query_TEST.py \
	environment_TEST.py \
	package_database_TEST.py \
	repository_TEST.py \
	version_metadata_TEST.py

IF_PYTHON_QA_TESTS =

IF_PYTHON_SOURCES = \
	exception.cc \
	contents.cc \
	dep_spec.cc \
	environment.cc \
	fs_entry.cc \
	mask_reasons.cc \
	name.cc \
	log.cc \
	package_database.cc \
	portage_dep_parser.cc \
	query.cc \
	repository.cc \
	version_metadata.cc \
	version_operator.cc \
	version_requirements.cc \
	version_spec.cc

IF_PYTHON_QA_SOURCES =

EXTRA_DIST = $(IF_PYTHON_TESTS) $(IF_PYTHON_SOURCES) \
	$(IF_PYTHON_QA_TESTS) $(IF_PYTHON_QA_SOURCES) \
	exception.hh \
	paludis_python.hh \
	paludis_python_so.cc \
	environment_TEST_setup.sh environment_TEST_cleanup.sh \
	package_database_TEST_setup.sh package_database_TEST_cleanup.sh \
	repository_TEST_setup.sh repository_TEST_cleanup.sh \
	version_metadata_TEST_setup.sh version_metadata_TEST_cleanup.sh

TESTS_ENVIRONMENT = env \
	PALUDIS_NO_GLOBAL_HOOKS="yes" \
	PALUDIS_NO_XTERM_TITLES="yes" \
	PALUDIS_EBUILD_DIR="`$(top_srcdir)/paludis/repositories/gentoo/ebuild/utils/canonicalise $(top_srcdir)/paludis/repositories/gentoo/ebuild/`" \
	PALUDIS_EBUILD_DIR_FALLBACK="`$(top_srcdir)/paludis/repositories/gentoo/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories/gentoo/ebuild/`" \
	PALUDIS_REPOSITORY_SO_DIR="`$(top_srcdir)/paludis/repositories/gentoo/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories`" \
	PALUDIS_ENVIRONMENT_SO_DIR="`$(top_srcdir)/paludis/repositories/gentoo/ebuild/utils/canonicalise $(top_builddir)/paludis/environments`" \
	SYSCONFDIR="$(sysconfdir)" \
	LD_LIBRARY_PATH=`echo "\`$(top_srcdir)/paludis/repositories/gentoo/ebuild/utils/canonicalise $(top_builddir)/paludis/.libs\`: \
			\`$(top_srcdir)/paludis/repositories/gentoo/ebuild/utils/canonicalise $(top_builddir)/paludis/util/.libs\`: \
			\`$(top_srcdir)/paludis/repositories/gentoo/ebuild/utils/canonicalise $(top_builddir)/paludis/environments/.libs\`: \
			\`$(top_srcdir)/paludis/repositories/gentoo/ebuild/utils/canonicalise $(top_builddir)/paludis/environments/paludis/.libs\`: \
			\`$(top_srcdir)/paludis/repositories/gentoo/ebuild/utils/canonicalise $(top_builddir)/paludis/environments/no_config/.libs\`: \
			\`$(top_srcdir)/paludis/repositories/gentoo/ebuild/utils/canonicalise $(top_builddir)/paludis/dep_list/.libs\`: \
			\`$(top_srcdir)/paludis/repositories/gentoo/ebuild/utils/canonicalise $(top_builddir)/paludis/tasks/.libs\`: \
			\`$(top_srcdir)/paludis/repositories/gentoo/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories/.libs\`: \
			\`$(top_srcdir)/paludis/repositories/gentoo/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories/gentoo/.libs\`: \
			\`$(top_srcdir)/paludis/repositories/gentoo/ebuild/utils/canonicalise $(top_builddir)/paludis/qa/.libs\`: \
			\`$(top_srcdir)/paludis/repositories/gentoo/ebuild/utils/canonicalise $(top_builddir)/python/.libs\`" \
			| tr -d ' '` \
	PYTHONPATH="./.libs" \
	bash $(top_srcdir)/test/run_test.sh "$(PYTHON)"

if ENABLE_PYTHON

lib_LTLIBRARIES = libpaludispython.la

libpaludispython_la_CXXFLAGS = $(AM_CXXFLAGS) -I. -I@PYTHON_INCLUDE_DIR@
libpaludispython_la_LDFLAGS = @BOOST_PYTHON_LIB@

paludis_python_so.o : paludis_python_so.cc
	if $(LIBTOOL) --tag=CXX --mode=compile $(CXX) $(DEFS) $(DEFAULT_INCLUDES) \
	    $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(CXXFLAGS) $(AM_CXXFLAGS) -I$(PYTHON_INCLUDE_DIR) -c \
	    -o $@ $(srcdir)/paludis_python_so.cc ; then ln -fs .libs/paludis_python_so.o $@ ; else rm -f $@ ; exit 1 ; fi

if ENABLE_QA

libpaludispython_la_SOURCES = $(IF_PYTHON_SOURCES) $(IF_PYTHON_QA_SOURCES)
libpaludispython_la_LIBADD = \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/dep_list/libpaludisdeplist.la \
	$(top_builddir)/paludis/tasks/libpaludistasks.la \
	$(top_builddir)/paludis/repositories/libpaludisrepositories.la \
	$(top_builddir)/paludis/repositories/gentoo/libpaludisgentoorepository.la \
	$(top_builddir)/paludis/environments/paludis/libpaludispaludisenvironment.la \
	$(top_builddir)/paludis/environments/no_config/libpaludisnoconfigenvironment.la \
	$(top_builddir)/paludis/environments/libpaludisenvironments.la \
	$(top_builddir)/paludis/qa/libpaludisqa.la

TESTS = $(IF_PYTHON_QA_TESTS) $(IF_PYTHON_TESTS)

else

libpaludispython_la_SOURCES = $(IF_PYTHON_SOURCES)
libpaludispython_la_LIBADD = \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/dep_list/libpaludisdeplist.la \
	$(top_builddir)/paludis/tasks/libpaludistasks.la \
	$(top_builddir)/paludis/repositories/libpaludisrepositories.la \
	$(top_builddir)/paludis/repositories/gentoo/libpaludisgentoorepository.la \
	$(top_builddir)/paludis/environments/paludis/libpaludispaludisenvironment.la \
	$(top_builddir)/paludis/environments/no_config/libpaludisnoconfigenvironment.la \
	$(top_builddir)/paludis/environments/libpaludisenvironments.la

TESTS = $(IF_PYTHON_TESTS)

endif

check_DATA = .libs/paludis.so
pythonlibdir = @PYTHON_INSTALL_DIR@
pythonlib_DATA = paludis.so

.libs/paludis.so : libpaludispython.la paludis_python_so.o
	mkdir -p .libs
	$(CXX) -fPIC -shared $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) \
	    $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) \
	    $(AM_LDFLAGS) $(LDFLAGS) \
	    -I@PYTHON_INCLUDE_DIR@ @BOOST_PYTHON_LIB@ \
	    -o $@ \
	    .libs/paludis_python_so.o \
	    -L$(top_builddir)/python/.libs -lpaludispython

paludis.so : libpaludispython.la paludis_python_so.o
	$(CXX) -fPIC -shared $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) \
	    $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) \
	    $(AM_LDFLAGS) $(LDFLAGS) \
	    -I@PYTHON_INCLUDE_DIR@ @BOOST_PYTHON_LIB@ \
	    -o $@ \
	    .libs/paludis_python_so.o \
	    -L$(top_builddir)/python/.libs -lpaludispython

endif

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done
