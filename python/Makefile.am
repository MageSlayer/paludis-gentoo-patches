SUBDIRS = .

AM_CXXFLAGS = -I$(top_srcdir) -I$(srcdir)/ \
	      -I$(top_builddir)/ \
	      @PALUDIS_CXXFLAGS@ \
	      @PALUDIS_CXXFLAGS_NO_WUNUSED@ \
	      @PALUDIS_CXXFLAGS_NO_WSHADOW@ \
	      @PALUDIS_CXXFLAGS_NO_WEXTRA@ \
	      @PALUDIS_CXXFLAGS_NO_WOLD_STYLE_CAST@ \
	      @PALUDIS_CXXFLAGS_NO_WWRITE_STRINGS@ \
	      @PALUDIS_CXXFLAGS_NO_WMISSING_NORETURN@ \
	      @PALUDIS_CXXFLAGS_VISIBILITY@ \
	      @PALUDIS_CXXFLAGS_NO_STRICT_ALIASING@

DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DLIBDIR=\"$(libdir)\"

CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda *.o paludis.so
MAINTAINERCLEANFILES = Makefile.in

IF_PYTHON_TESTS = \
	version_spec_TEST.py \
	version_operator_TEST.py \
	version_requirements_TEST.py \
	contents_TEST.py \
	dep_spec_TEST.py \
	dep_tag_TEST.py \
	log_TEST.py \
	name_TEST.py \
	query_TEST.py \
	environment_TEST.py \
	package_database_TEST.py \
	repository_TEST.py \
	action_TEST.py \
	metadata_key_TEST.py \
	mask_TEST.py \
	package_id_TEST.py \
	dep_list_TEST.py

IF_PYTHON_QA_TESTS = \
	qa_TEST.py

IF_PYTHON_SOURCES = \
	paludis_python.hh \
	options.hh \
	iterable.hh \
	validated.hh \
	action.cc \
	contents.cc \
	dep_list.cc \
	dep_spec.hh dep_spec.cc \
	dep_tag.cc \
	environment.cc \
	exception.hh exception.cc \
	fs_entry.cc \
	mask.cc \
	metadata_key.cc \
	mutex.hh mutex.cc \
	name.cc \
	log.cc \
	package_database.cc \
	package_id.cc \
	query.cc \
	repository.cc \
	version_operator.cc \
	version_requirements.cc \
	version_spec.cc

IF_PYTHON_QA_SOURCES = \
	qa.cc

EXTRA_DIST = $(IF_PYTHON_TESTS) $(IF_PYTHON_SOURCES) \
	$(IF_PYTHON_QA_TESTS) $(IF_PYTHON_QA_SOURCES) \
	paludis_python_so.cc \
	dep_list_TEST_setup.sh dep_list_TEST_cleanup.sh \
	environment_TEST_setup.sh environment_TEST_cleanup.sh \
	mask_TEST_setup.sh mask_TEST_cleanup.sh \
	metadata_key_TEST_setup.sh metadata_key_TEST_cleanup.sh \
	package_database_TEST_setup.sh package_database_TEST_cleanup.sh \
	package_id_TEST_setup.sh package_id_TEST_cleanup.sh \
	query_TEST_setup.sh query_TEST_cleanup.sh \
	repository_TEST_setup.sh repository_TEST_cleanup.sh

TESTS_ENVIRONMENT = env \
	TEST_OUTPUT_WRAPPER="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/util/outputwrapper`" \
	PALUDIS_OUTPUTWRAPPER_DIR="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/util/`" \
	PALUDIS_NO_GLOBAL_HOOKS="yes" \
	PALUDIS_NO_XTERM_TITLES="yes" \
	PALUDIS_EBUILD_DIR="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_srcdir)/paludis/repositories/e/ebuild/`" \
	PALUDIS_EBUILD_DIR_FALLBACK="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories/e/ebuild/`" \
	PALUDIS_EAPIS_DIR="$(top_srcdir)/paludis/repositories/e/eapis/" \
	PALUDIS_DISTRIBUTIONS_DIR="$(top_srcdir)/paludis/distributions/" \
	PALUDIS_DISTRIBUTION="gentoo" \
	PALUDIS_REPOSITORY_SO_DIR="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories`" \
	PALUDIS_ENVIRONMENT_SO_DIR="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/environments`" \
	SYSCONFDIR="$(sysconfdir)" \
	LD_LIBRARY_PATH=`echo "\`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/.libs\`: \
			\`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/util/.libs\`: \
			\`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/environments/.libs\`: \
			\`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/environments/paludis/.libs\`: \
			\`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/environments/no_config/.libs\`: \
			\`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/environments/adapted/.libs\`: \
			\`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/dep_list/.libs\`: \
			\`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/tasks/.libs\`: \
			\`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories/.libs\`: \
			\`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories/e/.libs\`: \
			\`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories/fake/.libs\`: \
			\`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/python/.libs\`" \
			| tr -d ' '` \
	PYTHONPATH="./.libs" \
	bash $(top_srcdir)/test/run_test.sh "$(PYTHON)"

if ENABLE_PYTHON

lib_LTLIBRARIES = libpaludispython.la

libpaludispython_la_CXXFLAGS = $(AM_CXXFLAGS) -I. -I@PYTHON_INCLUDE_DIR@
libpaludispython_la_LDFLAGS = @BOOST_PYTHON_LIB@

check_LTLIBRARIES = libadditionaltests.la

libadditionaltests_la_CXXFLAGS = $(AM_CXXFLAGS) -I. -I@PYTHON_INCLUDE_DIR@
libadditionaltests_la_LDFLAGS = @BOOST_PYTHON_LIB@ -rpath /nowhere

paludis_python_so.o : paludis_python_so.cc
	if $(LIBTOOL) --tag=CXX --mode=compile $(CXX) $(DEFS) $(DEFAULT_INCLUDES) \
	    $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(CXXFLAGS) $(AM_CXXFLAGS) -I$(PYTHON_INCLUDE_DIR) -c \
	    -o $@ $(srcdir)/paludis_python_so.cc ; then ln -fs .libs/paludis_python_so.o $@ ; else rm -f $@ ; exit 1 ; fi

additional_tests_so.o : additional_tests_so.cc
	if $(LIBTOOL) --tag=CXX --mode=compile $(CXX) $(DEFS) $(DEFAULT_INCLUDES) \
	    $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(CXXFLAGS) $(AM_CXXFLAGS) -I$(PYTHON_INCLUDE_DIR) -c \
	    -o $@ $(srcdir)/additional_tests_so.cc ; then ln -fs .libs/addtional_tests_so.o $@ ; else rm -f $@ ; exit 1 ; fi

if ENABLE_QA

libpaludispython_la_SOURCES = $(IF_PYTHON_SOURCES) $(IF_PYTHON_QA_SOURCES)
TESTS = $(IF_PYTHON_TESTS) $(IF_PYTHON_QA_TESTS)

else

libpaludispython_la_SOURCES = $(IF_PYTHON_SOURCES)
TESTS = $(IF_PYTHON_TESTS)

endif

libpaludispython_la_LIBADD = \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/dep_list/libpaludisdeplist.la \
	$(top_builddir)/paludis/tasks/libpaludistasks.la \
	$(top_builddir)/paludis/repositories/libpaludisrepositories.la \
	$(top_builddir)/paludis/repositories/e/libpaludiserepository.la \
	$(top_builddir)/paludis/repositories/fake/libpaludisfakerepository.la \
	$(top_builddir)/paludis/environments/paludis/libpaludispaludisenvironment.la \
	$(top_builddir)/paludis/environments/no_config/libpaludisnoconfigenvironment.la \
	$(top_builddir)/paludis/environments/adapted/libpaludisadaptedenvironment.la \
	$(top_builddir)/paludis/environments/test/libpaludistestenvironment.la \
	$(top_builddir)/paludis/environments/libpaludisenvironments.la

libadditionaltests_la_SOURCES = additional_tests.cc
libadditionaltests_la_LIBADD = $(libpaludispython_la_LIBADD)

check_DATA = .libs/paludis.so .libs/additional_tests.so
pythonlibdir = @PYTHON_INSTALL_DIR@
pythonlib_DATA = paludis.so

.libs/paludis.so : libpaludispython.la paludis_python_so.o
	mkdir -p .libs
	$(CXX) -fPIC -shared $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) \
	    $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) \
	    $(AM_LDFLAGS) $(LDFLAGS) \
	    -I@PYTHON_INCLUDE_DIR@ @BOOST_PYTHON_LIB@  $(PTHREAD_LIBS) \
	    -o $@ \
	    .libs/paludis_python_so.o \
	    -L$(top_builddir)/python/.libs -lpaludispython

paludis.so : libpaludispython.la paludis_python_so.o
	$(CXX) -fPIC -shared $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) \
	    $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) \
	    $(AM_LDFLAGS) $(LDFLAGS) \
	    -I@PYTHON_INCLUDE_DIR@ @BOOST_PYTHON_LIB@  $(PTHREAD_LIBS) \
	    -o $@ \
	    .libs/paludis_python_so.o \
	    -L$(top_builddir)/python/.libs -lpaludispython

.libs/additional_tests.so : libadditionaltests.la additional_tests_so.o
	$(CXX) -fPIC -shared $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) \
	    $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) \
	    $(AM_LDFLAGS) $(LDFLAGS) \
	    -I@PYTHON_INCLUDE_DIR@ @BOOST_PYTHON_LIB@  $(PTHREAD_LIBS) \
	    -o $@ \
	    .libs/additional_tests_so.o \
	    -L$(top_builddir)/python/.libs -ladditionaltests

endif

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done
