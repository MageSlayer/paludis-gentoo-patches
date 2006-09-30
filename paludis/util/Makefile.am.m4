ifdef(`__gnu__',`',`errprint(`This is not GNU m4...
')m4exit(1)') include(`misc/generated-file.txt')

dnl vim: set ft=m4 noet :

define(`filelist', `')dnl
define(`headerlist', `')dnl
define(`testlist', `')dnl
define(`testscriptlist', `')dnl
define(`addtest', `define(`testlist', testlist `$1_TEST')dnl
$1_TEST_SOURCES = $1_TEST.cc
$1_TEST_LDADD = test_extras.o $(top_builddir)/test/libtest.a libpaludisutil.la
$1_TEST_CXXFLAGS = -I$(top_srcdir)
')dnl
define(`addtestscript', `define(`testscriptlist', testscriptlist `$1_TEST_setup.sh $1_TEST_cleanup.sh')')dnl
define(`addhh', `define(`filelist', filelist `$1.hh')define(`headerlist', headerlist `$1.hh')')dnl
define(`addhhx', `define(`filelist', filelist `$1.hh')')dnl
define(`addcc', `define(`filelist', filelist `$1.cc')')dnl
define(`addimpl', `define(`filelist', filelist `$1-impl.hh')')dnl
define(`addthis', `dnl
ifelse(`$2', `hh', `addhh(`$1')', `')dnl
ifelse(`$2', `hhx', `addhhx(`$1')', `')dnl
ifelse(`$2', `cc', `addcc(`$1')', `')dnl
ifelse(`$2', `impl', `addimpl(`$1')', `')dnl
ifelse(`$2', `test', `addtest(`$1')', `')dnl
ifelse(`$2', `testscript', `addtestscript(`$1')', `')')dnl
define(`add', `addthis(`$1',`$2')addthis(`$1',`$3')addthis(`$1',`$4')dnl
addthis(`$1',`$5')addthis(`$1',`$6')')dnl

include(`paludis/util/files.m4')

CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda
MAINTAINERCLEANFILES = Makefile.in Makefile.am paludis.hh \
	hashed_containers.hh util.hh attributes.hh
AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
DEFS=\
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\"
EXTRA_DIST = util.hh.m4 Makefile.am.m4 files.m4 \
	testscriptlist test_extras.cc attributes.hh.in
SUBDIRS = .

libpaludisutil_la_SOURCES = filelist
libpaludisutil_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0

TESTS = testlist

TESTS_ENVIRONMENT = env \
	PALUDIS_EBUILD_DIR="$(srcdir)/ebuild/" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	bash $(top_srcdir)/test/run_test.sh

check_PROGRAMS = $(TESTS)
check_SCRIPTS = testscriptlist
lib_LTLIBRARIES = libpaludisutil.la
paludis_util_includedir = $(includedir)/paludis/util/
paludis_util_include_HEADERS = headerlist

Makefile.am : Makefile.am.m4 files.m4
	$(top_srcdir)/misc/do_m4.bash Makefile.am

util.hh : util.hh.m4 files.m4
	$(top_srcdir)/misc/do_m4.bash util.hh

