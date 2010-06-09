include $(top_srcdir)/misc/common-makefile.am

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
	      @PALUDIS_CXXFLAGS_NO_STRICT_ALIASING@ \
	      @PALUDIS_CXXFLAGS_NO_DEBUGGING@

DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DLIBDIR=\"$(libdir)\"

CLEANFILES += paludis.so
DISTCLEANFILES = *-nn.hh *-nn.cc

IF_PYTHON_TESTS = \
	action_TEST.py \
	contents_TEST.py \
	choices_TEST.py \
	dep_label_TEST.py \
	dep_list_TEST.py \
	dep_spec_TEST.py \
	dep_tag_TEST.py \
	environment_TEST.py \
	filter_TEST.py \
	filtered_generator_TEST.py \
	formatter_TEST.py \
	generator_TEST.py \
	log_TEST.py \
	mask_TEST.py \
	metadata_key_TEST.py \
	name_TEST.py \
	package_database_TEST.py \
	package_id_TEST.py \
	repository_TEST.py \
	selection_TEST.py \
	version_operator_TEST.py \
	version_requirements_TEST.py \
	version_spec_TEST.py

IF_PYTHON_SOURCES = \
	paludis_python.hh \
	options.hh \
	iterable.hh \
	wrapped_value.hh \
	about.cc \
	action.cc \
	choices.cc \
	contents.cc \
	dep_label.cc \
	dep_list.cc \
	dep_spec.hh dep_spec.cc \
	dep_tag.cc \
	environment.cc \
	exception.hh exception.cc \
	filter.cc \
	filtered_generator.cc \
	formatter.cc \
	fs_entry.cc \
	generator.cc \
	mask.cc \
	match_package.cc \
	metadata_key.cc \
	mutex.hh mutex.cc \
	name.cc \
	nice_names-nn.hh nice_names-nn.cc \
	log.cc \
	package_database.cc \
	package_id.cc \
	repository.cc \
	selection.cc \
	version_operator.cc \
	version_requirements.cc \
	version_spec.cc

BUILT_SOURCES = \
	nice_names-nn.hh nice_names-nn.cc

EXTRA_DIST = $(IF_PYTHON_TESTS) $(IF_PYTHON_SOURCES) \
	paludis_python_so.cc \
	additional_tests_so.cc \
	choices_TEST_setup.sh choices_TEST_cleanup.sh \
	dep_list_TEST_setup.sh dep_list_TEST_cleanup.sh \
	environment_TEST_setup.sh environment_TEST_cleanup.sh \
	mask_TEST_setup.sh mask_TEST_cleanup.sh \
	metadata_key_TEST_setup.sh metadata_key_TEST_cleanup.sh \
	nice_names.nn \
	package_database_TEST_setup.sh package_database_TEST_cleanup.sh \
	package_id_TEST_setup.sh package_id_TEST_cleanup.sh \
	repository_TEST_setup.sh repository_TEST_cleanup.sh \
	paludis_output_wrapper.py

if ENABLE_VIRTUALS_REPOSITORY
PALUDIS_ENABLE_VIRTUALS_REPOSITORY_VALUE=yes
else
PALUDIS_ENABLE_VIRTUALS_REPOSITORY_VALUE=no
endif

if ENABLE_PYTHON

lib_LTLIBRARIES = libpaludispython_@PALUDIS_PC_SLOT@.la

libpaludispython_@PALUDIS_PC_SLOT@_la_CXXFLAGS = $(AM_CXXFLAGS) -I. -I@PYTHON_INCLUDE_DIR@
libpaludispython_@PALUDIS_PC_SLOT@_la_LDFLAGS = \
	@BOOST_PYTHON_LIB@ \
	-version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0 \
	$(PTHREAD_LIBS)

check_LTLIBRARIES = libadditionaltests_@PALUDIS_PC_SLOT@.la

libadditionaltests_@PALUDIS_PC_SLOT@_la_CXXFLAGS = $(AM_CXXFLAGS) -I. -I@PYTHON_INCLUDE_DIR@
libadditionaltests_@PALUDIS_PC_SLOT@_la_LDFLAGS = @BOOST_PYTHON_LIB@ -rpath /nowhere

paludis_python_so.o : paludis_python_so.cc
	if $(LIBTOOL) --tag=CXX --mode=compile $(CXX) $(DEFS) $(DEFAULT_INCLUDES) \
	    $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(CXXFLAGS) $(AM_CXXFLAGS) -I$(PYTHON_INCLUDE_DIR) -c \
	    -o $@ $(srcdir)/paludis_python_so.cc ; then ln -fs .libs/paludis_python_so.o $@ ; else rm -f $@ ; exit 1 ; fi

additional_tests_so.o : additional_tests_so.cc
	if $(LIBTOOL) --tag=CXX --mode=compile $(CXX) $(DEFS) $(DEFAULT_INCLUDES) \
	    $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(CXXFLAGS) $(AM_CXXFLAGS) -I$(PYTHON_INCLUDE_DIR) -c \
	    -o $@ $(srcdir)/additional_tests_so.cc ; then ln -fs .libs/addtional_tests_so.o $@ ; else rm -f $@ ; exit 1 ; fi

libpaludispython_@PALUDIS_PC_SLOT@_la_SOURCES = $(IF_PYTHON_SOURCES)
TESTS = $(IF_PYTHON_TESTS)

libpaludispython_@PALUDIS_PC_SLOT@_la_LIBADD = \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la

libadditionaltests_@PALUDIS_PC_SLOT@_la_SOURCES = additional_tests.hh additional_tests.cc
libadditionaltests_@PALUDIS_PC_SLOT@_la_LIBADD = $(libpaludispython_@PALUDIS_PC_SLOT@_la_LIBADD)

check_DATA = .libs/paludis.so .libs/additional_tests.so
pythonlibdir = @PYTHON_INSTALL_DIR@
pythonlib_DATA = paludis.so paludis_output_wrapper.py

.libs/paludis.so : libpaludispython_@PALUDIS_PC_SLOT@.la paludis_python_so.o
	mkdir -p .libs
	$(CXX) -fPIC -shared $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) \
	    $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) \
	    $(AM_LDFLAGS) $(LDFLAGS) \
	    -I@PYTHON_INCLUDE_DIR@ @BOOST_PYTHON_LIB@  $(PTHREAD_LIBS) \
	    -o $@ \
	    .libs/paludis_python_so.o \
	    -L$(top_builddir)/python/.libs -lpaludispython_@PALUDIS_PC_SLOT@

paludis.so : libpaludispython_@PALUDIS_PC_SLOT@.la paludis_python_so.o
	$(CXX) -fPIC -shared $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) \
	    $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) \
	    $(AM_LDFLAGS) $(LDFLAGS) \
	    -I@PYTHON_INCLUDE_DIR@ @BOOST_PYTHON_LIB@  $(PTHREAD_LIBS) \
	    -o $@ \
	    .libs/paludis_python_so.o \
	    -L$(top_builddir)/python/.libs -lpaludispython_@PALUDIS_PC_SLOT@

.libs/additional_tests.so : libadditionaltests_@PALUDIS_PC_SLOT@.la additional_tests_so.o
	$(CXX) -fPIC -shared $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) \
	    $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) \
	    $(AM_LDFLAGS) $(LDFLAGS) \
	    -I@PYTHON_INCLUDE_DIR@ @BOOST_PYTHON_LIB@  $(PTHREAD_LIBS) \
	    -o $@ \
	    .libs/additional_tests_so.o \
	    -L$(top_builddir)/python/.libs -ladditionaltests_@PALUDIS_PC_SLOT@

endif

nice_names-nn.hh : nice_names.nn $(top_srcdir)/misc/make_nn.bash
	if ! $(top_srcdir)/misc/make_nn.bash --header $(srcdir)/nice_names.nn > $@ ; then rm -f $@ ; exit 1 ; fi

nice_names-nn.cc : nice_names.nn $(top_srcdir)/misc/make_nn.bash
	if ! $(top_srcdir)/misc/make_nn.bash --source $(srcdir)/nice_names.nn > $@ ; then rm -f $@ ; exit 1 ; fi

