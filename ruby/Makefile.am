SUBDIRS = . demos

AM_CXXFLAGS = -I$(top_srcdir) -I$(srcdir)/ \
	      -I$(top_builddir)/ \
	      @PALUDIS_CXXFLAGS_WITHOUT_PEDANTIC@ \
	      @PALUDIS_CXXFLAGS_NO_WREDUNDANT_DECLS@ \
	      @PALUDIS_CXXFLAGS_NO_WOLD_STYLE_CAST@

DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DLIBDIR=\"$(libdir)\"

CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda *.o *.so .libs/*.so
MAINTAINERCLEANFILES = Makefile.in

IF_RUBY_TESTS = \
	paludis_ruby_TEST.rb \
	log_TEST.rb \
	environment_TEST.rb \
	package_database_entry_TEST.rb \
	version_spec_TEST.rb \
	mask_reasons_TEST.rb \
	dep_atom_TEST.rb \
	package_database_TEST.rb \
	repository_TEST.rb \
	version_metadata_TEST.rb \
	portage_dep_parser_TEST.rb \
	qualified_package_name_TEST.rb \
	contents_TEST.rb \
	dep_list_TEST.rb

IF_RUBY_QA_TESTS = \
	message_TEST.rb \
	check_result_TEST.rb \
	metadata_file_TEST.rb \
	qa_environment_TEST.rb \
	check_TEST.rb

IF_RUBY_SOURCES = \
	paludis_ruby.cc paludis_ruby.hh \
	environment.cc \
	package_database_entry.cc \
	version_spec.cc \
	mask_reasons.cc \
	dep_atom.cc \
	log.cc \
	package_database.cc \
	repository.cc \
	version_metadata.cc \
	portage_dep_parser.cc \
	qualified_package_name.cc \
	contents.cc \
	dep_list.cc

IF_RUBY_QA_SOURCES = \
	message.cc \
	check.cc \
	check_result.cc \
	qa_environment.cc \
	metadata_file.cc

EXTRA_DIST = $(IF_RUBY_TESTS) $(IF_RUBY_SOURCES) \
	$(IF_RUBY_QA_TESTS) $(IF_RUBY_QA_SOURCES) \
	environment_TEST_setup.sh environment_TEST_cleanup.sh \
	package_database_TEST_setup.sh package_database_TEST_cleanup.sh \
	repository_TEST_setup.sh repository_TEST_cleanup.sh \
	version_metadata_TEST_setup.sh version_metadata_TEST_cleanup.sh \
	paludis_ruby_TEST_setup.sh paludis_ruby_TEST_cleanup.sh \
	qa_environment_TEST_setup.sh qa_environment_TEST_cleanup.sh \
	check_TEST_setup.sh check_TEST_cleanup.sh \
	dep_list_TEST_setup.sh dep_list_TEST_cleanup.sh

TESTS_ENVIRONMENT = env \
	PALUDIS_NO_GLOBAL_HOOKS="yes" \
	PALUDIS_NO_XTERM_TITLES="yes" \
	PALUDIS_EBUILD_DIR="`$(top_srcdir)/ebuild/utils/canonicalise $(top_srcdir)/ebuild/`" \
	PALUDIS_EBUILD_DIR_FALLBACK="`$(top_srcdir)/ebuild/utils/canonicalise $(top_builddir)/ebuild/`" \
	PALUDIS_REPOSITORY_SO_DIR="`$(top_srcdir)/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories`" \
	SYSCONFDIR="$(sysconfdir)" \
	LD_LIBRARY_PATH=`echo "\`$(top_srcdir)/ebuild/utils/canonicalise $(top_builddir)/paludis/.libs\`: \
			\`$(top_srcdir)/ebuild/utils/canonicalise $(top_builddir)/paludis/util/.libs\`: \
			\`$(top_srcdir)/ebuild/utils/canonicalise $(top_builddir)/paludis/environment/default/.libs\`: \
			\`$(top_srcdir)/ebuild/utils/canonicalise $(top_builddir)/paludis/environment/no_config/.libs\`: \
			\`$(top_srcdir)/ebuild/utils/canonicalise $(top_builddir)/paludis/dep_list/.libs\`: \
			\`$(top_srcdir)/ebuild/utils/canonicalise $(top_builddir)/paludis/qa/.libs\`: \
			\`$(top_srcdir)/ebuild/utils/canonicalise $(top_builddir)/paludis/libxml/.libs\`" \
			| tr -d ' '` \
	bash $(top_srcdir)/test/run_test.sh "$(RUBY) -I ./.libs/"

if ENABLE_RUBY

BASE_OBJECTS = \
	environment.o \
	dep_atom.o \
	log.o \
	mask_reasons.o \
	package_database.o \
	package_database_entry.o \
	version_spec.o \
	repository.o \
	version_metadata.o \
	portage_dep_parser.o \
	qualified_package_name.o \
	contents.o \
	dep_list.o \
	paludis_ruby.o

if ENABLE_QA

OUR_OBJECTS = $(BASE_OBJECTS)\
	paludis_ruby.o \
	message.o \
	check.o \
	check_result.o \
	qa_environment.o \
	metadata_file.o

TESTS = $(IF_RUBY_TESTS) $(IF_RUBY_QA_TESTS)

else

OUR_OBJECTS = $(BASE_OBJECTS)
TESTS = $(IF_RUBY_TESTS)

endif

noinst_DATA = $(OUR_OBJECTS)
check_DATA = .libs/Paludis.so
rubylibdir = @RUBY_INSTALL_DIR@
rubylib_DATA = Paludis.so

OUR_CXXCOMPILE = $(CXX) -fPIC $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) \
	    $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) \
	    -I. -I@RUBY_SITEARCHDIR@ -I@RUBY_ARCHDIR@ -c

paludis_ruby.o : paludis_ruby.cc paludis_ruby.hh
	$(OUR_CXXCOMPILE) -o $@ $(srcdir)/paludis_ruby.cc

environment.o : environment.cc paludis_ruby.hh
	$(OUR_CXXCOMPILE) -o $@ $(srcdir)/environment.cc

dep_atom.o : dep_atom.cc paludis_ruby.hh
	$(OUR_CXXCOMPILE) -o $@ $(srcdir)/dep_atom.cc

package_database_entry.o : package_database_entry.cc paludis_ruby.hh
	$(OUR_CXXCOMPILE) -o $@ $(srcdir)/package_database_entry.cc

package_database.o : package_database.cc paludis_ruby.hh
	$(OUR_CXXCOMPILE) -o $@ $(srcdir)/package_database.cc

version_spec.o : version_spec.cc paludis_ruby.hh
	$(OUR_CXXCOMPILE) -o $@ $(srcdir)/version_spec.cc

version_metadata.o : version_metadata.cc paludis_ruby.hh
	$(OUR_CXXCOMPILE) -o $@ $(srcdir)/version_metadata.cc

mask_reasons.o : mask_reasons.cc paludis_ruby.hh
	$(OUR_CXXCOMPILE) -o $@ $(srcdir)/mask_reasons.cc

log.o : log.cc paludis_ruby.hh
	$(OUR_CXXCOMPILE) -o $@ $(srcdir)/log.cc

repository.o : repository.cc paludis_ruby.hh
	$(OUR_CXXCOMPILE) -o $@ $(srcdir)/repository.cc

portage_dep_parser.o : portage_dep_parser.cc paludis_ruby.hh
	$(OUR_CXXCOMPILE) -o $@ $(srcdir)/portage_dep_parser.cc

qualified_package_name.o : qualified_package_name.cc paludis_ruby.hh
	$(OUR_CXXCOMPILE) -o $@ $(srcdir)/qualified_package_name.cc

contents.o : contents.cc paludis_ruby.hh
	$(OUR_CXXCOMPILE) -o $@ $(srcdir)/contents.cc

dep_list.o : dep_list.cc paludis_ruby.hh
	$(OUR_CXXCOMPILE) -o $@ $(srcdir)/dep_list.cc

message.o : message.cc paludis_ruby.hh
	$(OUR_CXXCOMPILE) -o $@ $(srcdir)/message.cc

check.o : check.cc paludis_ruby.hh
	$(OUR_CXXCOMPILE) -o $@ $(srcdir)/check.cc

check_result.o : check_result.cc paludis_ruby.hh
	$(OUR_CXXCOMPILE) -o $@ $(srcdir)/check_result.cc

qa_environment.o : qa_environment.cc paludis_ruby.hh
	$(OUR_CXXCOMPILE) -o $@ $(srcdir)/qa_environment.cc

metadata_file.o : metadata_file.cc paludis_ruby.hh
	$(OUR_CXXCOMPILE) -o $@ $(srcdir)/metadata_file.cc

if ENABLE_QA

.libs/Paludis.so : $(OUR_OBJECTS)
	mkdir -p .libs
	$(CXX) -fPIC -shared $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) \
	    $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) \
	    $(AM_LDFLAGS) $(LDFLAGS) \
	    -I@RUBY_SITEARCHDIR@ -l@RUBY_SONAME@ \
	    -o $@ \
	    $^ \
	    $(top_builddir)/paludis/util/.libs/libpaludisutil.so \
	    $(top_builddir)/paludis/.libs/libpaludis.so \
	    $(top_builddir)/paludis/environment/default/.libs/libpaludisdefaultenvironment.so \
	    $(top_builddir)/paludis/environment/no_config/.libs/libpaludisnoconfigenvironment.so \
	    $(top_builddir)/paludis/dep_list/.libs/libpaludisdeplist.so \
	    $(top_builddir)/paludis/qa/.libs/libpaludisqa.so \
	    $(top_builddir)/paludis/libxml/.libs/libpaludislibxml.so \
	    $(PCREPLUSPLUS_LIBS)

Paludis.so : $(OUR_OBJECTS)
	$(CXX) -fPIC -shared $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) \
	    $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) \
	    $(AM_LDFLAGS) $(LDFLAGS) \
	    -I@RUBY_SITEARCHDIR@ -l@RUBY_SONAME@ \
	    -o $@ \
	    $^ \
	    -L$(top_builddir)/paludis/util/.libs/ -lpaludisutil \
	    -L$(top_builddir)/paludis/.libs/ -lpaludis \
	    -L$(top_builddir)/paludis/environment/default/.libs/ -lpaludisdefaultenvironment \
	    -L$(top_builddir)/paludis/environment/no_config/.libs/ -lpaludisnoconfigenvironment \
	    -L$(top_builddir)/paludis/dep_list/.libs -lpaludisdeplist \
	    -L$(top_builddir)/paludis/qa/.libs/ -lpaludisqa \
	    -L$(top_builddir)/paludis/libxml/.libs/ -lpaludislibxml \
	    $(PCREPLUSPLUS_LIBS)

else

.libs/Paludis.so : $(OUR_OBJECTS)
	mkdir -p .libs
	$(CXX) -fPIC -shared $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) \
	    $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) \
	    $(AM_LDFLAGS) $(LDFLAGS) \
	    -I@RUBY_SITEARCHDIR@ -l@RUBY_SONAME@ \
	    -o $@ \
	    $^ \
	    $(top_builddir)/paludis/util/.libs/libpaludisutil.so \
	    $(top_builddir)/paludis/.libs/libpaludis.so \
	    $(top_builddir)/paludis/environment/default/.libs/libpaludisdefaultenvironment.so \
	    $(top_builddir)/paludis/environment/no_config/.libs/libpaludisnoconfigenvironment.so \
	    $(top_builddir)/paludis/dep_list/.libs/libpaludisdeplist.so

Paludis.so : $(OUR_OBJECTS)
	$(CXX) -fPIC -shared $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) \
	    $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) \
	    $(AM_LDFLAGS) $(LDFLAGS) \
	    -I@RUBY_SITEARCHDIR@ -l@RUBY_SONAME@ \
	    -o $@ \
	    $^ \
	    -L$(top_builddir)/paludis/util/.libs/ -lpaludisutil \
	    -L$(top_builddir)/paludis/.libs/ -lpaludis \
	    -L$(top_builddir)/paludis/environment/default/.libs/ -lpaludisdefaultenvironment \
	    -L$(top_builddir)/paludis/environment/no_config/.libs/ -lpaludisnoconfigenvironment \
	    -L$(top_builddir)/paludis/dep_list/.libs -lpaludisdeplist

endif

endif

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done


