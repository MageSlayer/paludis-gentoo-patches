SUBDIRS = eapis ebuild qa .
CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda
DISTCLEANFILES = \
	glsa-sr.hh glsa-sr.cc \
	e_repository_params-sr.hh e_repository_params-sr.cc \
	vdb_repository-sr.hh vdb_repository-sr.cc \
	ebuild-sr.hh ebuild-sr.cc \
	ebin-sr.hh ebin-sr.cc ebin-se.hh ebin-se.cc \
	vdb_merger-sr.hh vdb_merger-sr.cc \
	vdb_unmerger-sr.hh vdb_unmerger-sr.cc \
	eapi-sr.hh eapi-sr.cc \
	dep_parser-se.hh dep_parser-se.cc \
	manifest2_entry-sr.hh manifest2_entry-sr.cc

MAINTAINERCLEANFILES = Makefile.in

AM_CXXFLAGS = -I$(top_srcdir) -I$(top_builddir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBDIR=\"$(libdir)\"

paludis_repositories_libdir = $(libdir)/paludis/repositories
paludis_repositories_lib_LTLIBRARIES = libpaludiserepository.la

if ENABLE_GLSA

lib_LTLIBRARIES = libpaludiserepositoryxmlthings.la

endif

paludis_repositories_e_includedir = $(includedir)/paludis-$(PALUDIS_PC_SLOT)/paludis/repositories/e/
libpaludiserepository_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0

paludis_repositories_e_include_HEADERS = \
	aa_visitor.hh \
	check_fetched_files_visitor.hh \
	dep_lexer.hh \
	dep_parser.hh \
	dep_parser-se.hh \
	dep_parser-fwd.hh \
	dep_spec_pretty_printer.hh \
	e_key.hh \
	e_mask.hh \
	e_repository.hh \
	e_repository_entries.hh \
	e_repository_exceptions.hh \
	e_repository_id.hh \
	e_repository_mask_file.hh \
	e_repository_news.hh \
	e_repository_params-sr.hh \
	e_repository_params.hh \
	e_repository_profile.hh \
	e_repository_profile_file.hh \
	e_repository_sets.hh \
	eapi.hh \
	eapi-sr.hh \
	eapi-fwd.hh \
	eapi_phase.hh \
	ebin-se.hh \
	ebin-sr.hh \
	ebin.hh \
	ebin_entries.hh \
	ebuild-sr.hh \
	ebuild.hh \
	ebuild_entries.hh \
	ebuild_flat_metadata_cache.hh \
	ebuild_id.hh \
	eclass_mtimes.hh \
	exheres_layout.hh \
	fetch_visitor.hh \
	glsa-sr.hh \
	glsa.hh \
	layout.hh \
	make_ebin_repository.hh \
	make_ebuild_repository.hh \
	manifest2_entry-sr.hh \
	manifest2_reader.hh \
	source_uri_finder.hh \
	traditional_layout.hh \
	use_desc.hh \
	vdb_id.hh \
	vdb_merger-sr.hh \
	vdb_merger.hh \
	vdb_repository-sr.hh \
	vdb_repository.hh \
	vdb_unmerger-sr.hh \
	vdb_unmerger.hh

libpaludiserepository_la_SOURCES = \
	aa_visitor.cc \
	dep_lexer.cc \
	check_fetched_files_visitor.cc \
	dep_parser.cc \
	dep_spec_pretty_printer.cc \
	e_key.cc \
	e_mask.cc \
	e_repository.cc \
	e_repository_entries.cc \
	e_repository_exceptions.cc \
	e_repository_id.cc \
	e_repository_mask_file.cc \
	e_repository_news.cc \
	e_repository_params.cc \
	e_repository_profile.cc \
	e_repository_profile_file.cc \
	e_repository_sets.cc \
	eapi.cc \
	eapi_phase.cc \
	ebin.cc \
	ebin_entries.cc \
	ebuild.cc \
	ebuild_entries.cc \
	ebuild_flat_metadata_cache.cc \
	ebuild_id.cc \
	eclass_mtimes.cc \
	exheres_layout.cc \
	fetch_visitor.cc \
	glsa.cc \
	layout.cc \
	make_ebin_repository.cc \
	make_ebuild_repository.cc \
	manifest2_reader.cc \
	registration.cc \
	source_uri_finder.cc \
	traditional_layout.cc \
	use_desc.cc \
	vdb_id.cc \
	vdb_merger.cc \
	vdb_repository.cc \
	vdb_unmerger.cc \
	$(paludis_repositories_e_include_HEADERS)

if ENABLE_QA
libpaludiserepository_la_LIBADD_IF_QA = $(top_builddir)/paludis/repositories/e/qa/libpaludiserepositoryqa.la
else
libpaludiserepository_la_LIBADD_IF_QA =
endif

libpaludiserepository_la_LIBADD = \
	$(libpaludiserepository_la_LIBADD_IF_QA) \
	$(top_builddir)/paludis/repositories/libpaludisrepositories.la \
	$(top_builddir)/paludis/merger/libpaludismerger.la \
	$(top_builddir)/paludis/digests/libpaludisdigests.la \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(DYNAMIC_LD_LIBS)

e_repository_TEST_SOURCES = e_repository_TEST.cc

e_repository_TEST_LDADD = \
	libpaludiserepository.la \
	$(top_builddir)/paludis/repositories/fake/libpaludisfakerepository.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/environments/test/libpaludistestenvironment.la \
	$(top_builddir)/test/libtest.a \
	$(DYNAMIC_LD_LIBS)

e_repository_TEST_CXXFLAGS = $(AM_CXXFLAGS) -I$(top_srcdir)

vdb_merger_TEST_SOURCES = vdb_merger_TEST.cc

vdb_merger_TEST_LDADD = \
	libpaludiserepository.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/environments/test/libpaludistestenvironment.la \
	$(top_builddir)/test/libtest.a \
	$(DYNAMIC_LD_LIBS)

vdb_merger_TEST_CXXFLAGS = $(AM_CXXFLAGS) -I$(top_srcdir)

vdb_unmerger_TEST_SOURCES = vdb_unmerger_TEST.cc

vdb_unmerger_TEST_LDADD = \
	libpaludiserepository.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/environments/test/libpaludistestenvironment.la \
	$(top_builddir)/test/libtest.a \
	$(DYNAMIC_LD_LIBS)

vdb_unmerger_TEST_CXXFLAGS = $(AM_CXXFLAGS) -I$(top_srcdir)

e_repository_sets_TEST_SOURCES = e_repository_sets_TEST.cc

e_repository_sets_TEST_LDADD = \
	libpaludiserepository.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/environments/test/libpaludistestenvironment.la \
	$(top_builddir)/paludis/repositories/fake/libpaludisfakerepository.la \
	$(top_builddir)/test/libtest.a \
	$(DYNAMIC_LD_LIBS)

e_repository_sets_TEST_CXXFLAGS = $(AM_CXXFLAGS) -I$(top_srcdir)

dep_spec_pretty_printer_TEST_SOURCES = dep_spec_pretty_printer_TEST.cc

dep_spec_pretty_printer_TEST_LDADD = \
	libpaludiserepository.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/environments/test/libpaludistestenvironment.la \
	$(top_builddir)/test/libtest.a \
	$(DYNAMIC_LD_LIBS)

dep_lexer_TEST_SOURCES = dep_lexer_TEST.cc

dep_lexer_TEST_LDADD = \
	libpaludiserepository.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/environments/test/libpaludistestenvironment.la \
	$(top_builddir)/test/libtest.a \
	$(DYNAMIC_LD_LIBS)

dep_parser_TEST_SOURCES = dep_parser_TEST.cc

dep_parser_TEST_LDADD = \
	libpaludiserepository.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/environments/test/libpaludistestenvironment.la \
	$(top_builddir)/test/libtest.a \
	$(DYNAMIC_LD_LIBS)

aa_visitor_TEST_SOURCES = aa_visitor_TEST.cc

aa_visitor_TEST_LDADD = \
	libpaludiserepository.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/environments/test/libpaludistestenvironment.la \
	$(top_builddir)/test/libtest.a \
	$(DYNAMIC_LD_LIBS)

fetch_visitor_TEST_SOURCES = fetch_visitor_TEST.cc

fetch_visitor_TEST_LDADD = \
	libpaludiserepository.la \
	$(top_builddir)/paludis/repositories/fake/libpaludisfakerepository.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/environments/test/libpaludistestenvironment.la \
	$(top_builddir)/test/libtest.a \
	$(DYNAMIC_LD_LIBS)

source_uri_finder_TEST_SOURCES = source_uri_finder_TEST.cc

source_uri_finder_TEST_LDADD = \
	libpaludiserepository.la \
	$(top_builddir)/paludis/repositories/fake/libpaludisfakerepository.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/environments/test/libpaludistestenvironment.la \
	$(top_builddir)/test/libtest.a \
	$(DYNAMIC_LD_LIBS)

EXTRA_DIST = \
	aa_visitor_TEST.cc \
	dep_lexer_TEST.cc \
	dep_parser.se \
	dep_parser-se.hh \
	dep_parser-se.cc \
	dep_parser_TEST.cc \
	dep_spec_pretty_printer_TEST.cc \
	eapi.sr \
	eapi-sr.hh \
	eapi-sr.cc \
	ebuild.sr \
	ebuild-sr.hh \
	ebuild-sr.cc \
	ebin.sr \
	ebin-sr.hh \
	ebin-sr.cc \
	ebin.se \
	ebin-se.hh \
	ebin-se.cc \
	glsa.sr \
	glsa-sr.hh \
	glsa-sr.cc \
	e_repository_TEST.cc \
	e_repository_TEST_setup.sh \
	e_repository_TEST_cleanup.sh \
	e_repository_params-sr.hh \
	e_repository_params-sr.cc \
	e_repository_params.sr \
	e_repository_sets_TEST.cc \
	e_repository_sets_TEST_setup.sh \
	e_repository_sets_TEST_cleanup.sh \
	fetch_visitor_TEST.cc \
	fetch_visitor_TEST_setup.sh \
	fetch_visitor_TEST_cleanup.sh \
	manifest2_entry.sr \
	manifest2_entry-sr.hh \
	manifest2_entry-sr.cc \
	source_uri_finder_TEST.cc \
	xml_things_TEST.cc \
	xml_things_TEST_setup.sh \
	xml_things_TEST_cleanup.sh \
	vdb_repository_TEST.cc \
	vdb_repository_TEST_setup.sh \
	vdb_repository_TEST_cleanup.sh \
	vdb_repository-sr.hh \
	vdb_repository-sr.cc \
	vdb_repository.sr \
	vdb_merger.sr \
	vdb_merger-sr.hh \
	vdb_merger-sr.cc \
	vdb_unmerger.sr \
	vdb_unmerger-sr.hh \
	vdb_unmerger-sr.cc \
	vdb_merger_TEST.cc \
	vdb_merger_TEST_setup.sh \
	vdb_merger_TEST_cleanup.sh \
	vdb_unmerger_TEST.cc \
	vdb_unmerger_TEST_setup.sh \
	vdb_unmerger_TEST_cleanup.sh

BUILT_SOURCES = \
	dep_parser-se.hh \
	dep_parser-se.cc \
	e_repository_params-sr.hh \
	e_repository_params-sr.cc \
	glsa-sr.hh \
	glsa-sr.cc \
	vdb_repository-sr.hh \
	vdb_repository-sr.cc \
	ebuild-sr.hh \
	ebuild-sr.cc \
	ebin-sr.hh \
	ebin-sr.cc \
	ebin-se.hh \
	ebin-se.cc \
	vdb_merger-sr.hh \
	vdb_merger-sr.cc \
	vdb_unmerger-sr.hh \
	vdb_unmerger-sr.cc \
	eapi-sr.hh \
	eapi-sr.cc \
	manifest2_entry-sr.hh \
	manifest2_entry-sr.cc

check_SCRIPTS = \
	e_repository_TEST_setup.sh e_repository_TEST_cleanup.sh \
	xml_things_TEST_setup.sh xml_things_TEST_cleanup.sh \
	vdb_repository_TEST_setup.sh vdb_repository_TEST_cleanup.sh \
	e_repository_sets_TEST_setup.sh e_repository_sets_TEST_cleanup.sh \
	fetch_visitor_TEST_setup.sh fetch_visitor_TEST_cleanup.sh

TESTS_ENVIRONMENT = env \
	TEST_OUTPUT_WRAPPER="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/util/outputwrapper`" \
	PALUDIS_OUTPUTWRAPPER_DIR="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/util/`" \
	PALUDIS_EBUILD_DIR="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_srcdir)/paludis/repositories/e/ebuild/`" \
	PALUDIS_EAPIS_DIR="$(top_srcdir)/paludis/repositories/e/eapis/" \
	PALUDIS_DISTRIBUTIONS_DIR="$(top_srcdir)/paludis/distributions/" \
	PALUDIS_DISTRIBUTION="gentoo" \
	PALUDIS_FETCHERS_DIR="$(top_srcdir)/paludis/fetchers/" \
	PALUDIS_SKIP_CONFIG="yes" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	PALUDIS_REPOSITORY_SO_DIR="$(top_builddir)/paludis/repositories" \
	PALUDIS_NO_CHOWN="yes" \
	LD_LIBRARY_PATH="`echo $$LD_LIBRARY_PATH: | sed -e 's,^:,,'`` \
		$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories/e/`:` \
		$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories/e/.libs/`" \
	bash $(top_srcdir)/test/run_test.sh

e_repository_params-sr.hh : e_repository_params.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --header $(srcdir)/e_repository_params.sr > $@

e_repository_params-sr.cc : e_repository_params.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --source $(srcdir)/e_repository_params.sr > $@

manifest2_entry-sr.hh : manifest2_entry.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --header $(srcdir)/manifest2_entry.sr > $@

manifest2_entry-sr.cc : manifest2_entry.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --source $(srcdir)/manifest2_entry.sr > $@

vdb_merger-sr.hh : vdb_merger.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --header $(srcdir)/vdb_merger.sr > $@

vdb_merger-sr.cc : vdb_merger.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --source $(srcdir)/vdb_merger.sr > $@

vdb_unmerger-sr.hh : vdb_unmerger.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --header $(srcdir)/vdb_unmerger.sr > $@

vdb_unmerger-sr.cc : vdb_unmerger.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --source $(srcdir)/vdb_unmerger.sr > $@

ebuild-sr.hh : ebuild.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --header $(srcdir)/ebuild.sr > $@

ebuild-sr.cc : ebuild.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --source $(srcdir)/ebuild.sr > $@

ebin-sr.hh : ebin.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --header $(srcdir)/ebin.sr > $@

ebin-sr.cc : ebin.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --source $(srcdir)/ebin.sr > $@

eapi-sr.hh : eapi.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --header $(srcdir)/eapi.sr > $@

eapi-sr.cc : eapi.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --source $(srcdir)/eapi.sr > $@

glsa-sr.hh : glsa.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --header $(srcdir)/glsa.sr > $@

glsa-sr.cc : glsa.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --source $(srcdir)/glsa.sr > $@

vdb_repository-sr.hh : vdb_repository.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --header $(srcdir)/vdb_repository.sr > $@

vdb_repository-sr.cc : vdb_repository.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --source $(srcdir)/vdb_repository.sr > $@

ebin-se.hh : ebin.se $(top_srcdir)/misc/make_se.bash
	if ! $(top_srcdir)/misc/make_se.bash --header $(srcdir)/ebin.se > $@ ; then rm -f $@ ; exit 1 ; fi

ebin-se.cc : ebin.se $(top_srcdir)/misc/make_se.bash
	if ! $(top_srcdir)/misc/make_se.bash --source $(srcdir)/ebin.se > $@ ; then rm -f $@ ; exit 1 ; fi

dep_parser-se.hh : dep_parser.se $(top_srcdir)/misc/make_se.bash
	if ! $(top_srcdir)/misc/make_se.bash --header $(srcdir)/dep_parser.se > $@ ; then rm -f $@ ; exit 1 ; fi

dep_parser-se.cc : dep_parser.se $(top_srcdir)/misc/make_se.bash
	if ! $(top_srcdir)/misc/make_se.bash --source $(srcdir)/dep_parser.se > $@ ; then rm -f $@ ; exit 1 ; fi


libpaludiserepositoryxmlthings_la_SOURCES = xml_things.cc xml_things.hh
libpaludiserepositoryxmlthings_la_CXXFLAGS = $(AM_CXXFLAGS) @LIBXML2DEPS_CFLAGS@

libpaludiserepositoryxmlthings_la_LIBADD = @LIBXML2DEPS_LIBS@ \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/libpaludis.la

libpaludiserepositoryxmlthings_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0

if ENABLE_GLSA

GLSA_TESTS = xml_things_TEST
xml_things_TEST_SOURCES = xml_things_TEST.cc

xml_things_TEST_LDADD = \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/test/libtest.a \
	libpaludiserepository.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/environments/test/libpaludistestenvironment.la \
	$(top_builddir)/test/libtest.a \
	$(DYNAMIC_LD_LIBS)

endif

TESTS = \
	dep_lexer_TEST \
	dep_parser_TEST \
	dep_spec_pretty_printer_TEST \
	e_repository_TEST \
	vdb_merger_TEST \
	vdb_unmerger_TEST \
	$(GLSA_TESTS) \
	e_repository_sets_TEST \
	aa_visitor_TEST \
	fetch_visitor_TEST \
	source_uri_finder_TEST

check_PROGRAMS = $(TESTS)

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

