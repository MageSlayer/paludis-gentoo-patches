ifdef(`__gnu__',`',`errprint(`This is not GNU m4...
')m4exit(1)') include(`misc/generated-file.txt')

dnl vim: set ft=m4 noet :

define(`filelist', `')dnl
define(`testlist', `')dnl
define(`headerlist', `')dnl
define(`srlist', `')dnl
define(`srcleanlist', `')dnl
define(`srheaderlist', `')dnl
define(`testscriptlist', `')dnl
define(`addtest', `define(`testlist', testlist `$1_TEST')dnl
$1_TEST_SOURCES = $1_TEST.cc
$1_TEST_LDADD = \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/test/libtest.a \
	libpaludisqa.la \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/libxml/libpaludislibxml.la \
	$(top_builddir)/paludis/repositories/portage/libpaludisportagerepository.la \
	$(top_builddir)/paludis/repositories/virtuals/libpaludisvirtualsrepository.la \
	$(DYNAMIC_LD_LIBS) \
	$(PCREPLUSPLUS_LIBS) \
	$(LIBXML2DEPS_LIBS)
$1_TEST_CXXFLAGS = -I$(top_srcdir)
')dnl
define(`addtestscript', `define(`testscriptlist', testscriptlist `$1_TEST_setup.sh $1_TEST_cleanup.sh')')dnl
define(`addhh', `define(`filelist', filelist `$1.hh')define(`headerlist', headerlist `$1.hh')')dnl
define(`addcc', `define(`filelist', filelist `$1.cc')')dnl
define(`addimpl', `define(`filelist', filelist `$1-impl.hh')')dnl
define(`addsr', `define(`srlist', srlist `$1.sr')dnl
define(`srcleanlist', srcleanlist `$1-sr.hh $1-sr.cc')dnl
define(`srheaderlist', srheaderlist `$1-sr.hh')dnl
$1-sr.hh : $1.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --header $`'(srcdir)/$1.sr > $`'@

$1-sr.cc : $1.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --source $`'(srcdir)/$1.sr > $`'@

')dnl
define(`addthis', `dnl
ifelse(`$2', `hh', `addhh(`$1')', `')dnl
ifelse(`$2', `cc', `addcc(`$1')', `')dnl
ifelse(`$2', `sr', `addsr(`$1')', `')dnl
ifelse(`$2', `impl', `addimpl(`$1')', `')dnl
ifelse(`$2', `test', `addtest(`$1')', `')dnl
ifelse(`$2', `testscript', `addtestscript(`$1')', `')')dnl
define(`add', `addthis(`$1',`$2')addthis(`$1',`$3')addthis(`$1',`$4')dnl
addthis(`$1',`$5')addthis(`$1',`$6')')dnl

include(`paludis/qa/files.m4')

if ENABLE_QA

INCLUDES = $(PCREPLUSPLUS_CFLAGS) $(LIBXML2DEPS_CFLAGS)

libpaludisqa_la_SOURCES = filelist
libpaludisqa_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0
libpaludisqa_la_LIBADD = \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/libxml/libpaludislibxml.la \
	@LIBXML2DEPS_LIBS@

TESTS = testlist

TESTS_ENVIRONMENT = env \
	PALUDIS_EBUILD_DIR="$(top_srcdir)/ebuild/" \
	PALUDIS_SKIP_CONFIG="yes" \
	PALUDIS_REPOSITORY_SO_DIR="$(top_builddir)/paludis/repositories" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	bash $(top_srcdir)/test/run_test.sh

check_PROGRAMS = $(TESTS)
check_SCRIPTS = testscriptlist
lib_LTLIBRARIES = libpaludisqa.la
paludis_qaincludedir = $(includedir)/paludis/qa/
paludis_qainclude_HEADERS = headerlist srheaderlist

endif

Makefile.am : Makefile.am.m4 files.m4
	$(top_srcdir)/misc/do_m4.bash Makefile.am

qa.hh : qa.hh.m4 files.m4
	$(top_srcdir)/misc/do_m4.bash qa.hh

CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda
MAINTAINERCLEANFILES = Makefile.in Makefile.am qa.hh
DISTCLEANFILES = srcleanlist
BUILT_SOURCES = srcleanlist qa.hh
AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DBIGTEMPDIR=\"/var/tmp\"
EXTRA_DIST = Makefile.am.m4 files.m4 qa.hh.m4 testscriptlist srlist srcleanlist

built-sources : $(BUILT_SOURCES)
	for s in $(SUBDIRS) ; do $(MAKE) -C $$s built-sources || exit 1 ; done

