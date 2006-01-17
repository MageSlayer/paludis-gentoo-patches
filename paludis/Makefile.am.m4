ifdef(`__gnu__',`',`errprint(`This is not GNU m4...
')m4exit(1)') include(`misc/generated-file.txt')

dnl vim: set ft=m4 et :

define(`filelist', `')dnl
define(`testlist', `')dnl
define(`testscriptlist', `')dnl
define(`addtest', `define(`testlist', testlist `$1_TEST')dnl
$1_TEST_SOURCES = $1_TEST.cc
$1_TEST_LDADD = $(top_builddir)/test/libtest.a libpaludis.a
')dnl
define(`addtestscript', `define(`testscriptlist', testscriptlist `$1_TEST_setup.sh $1_TEST_cleanup.sh')')dnl
define(`addhh', `define(`filelist', filelist `$1.hh')')dnl
define(`addcc', `define(`filelist', filelist `$1.cc')')dnl
define(`addimpl', `define(`filelist', filelist `$1-impl.hh')')dnl
define(`addthis', `dnl
ifelse(`$2', `hh', `addhh(`$1')', `')dnl
ifelse(`$2', `cc', `addcc(`$1')', `')dnl
ifelse(`$2', `impl', `addimpl(`$1')', `')dnl
ifelse(`$2', `test', `addtest(`$1')', `')dnl
ifelse(`$2', `testscript', `addtestscript(`$1')', `')')dnl
define(`add', `addthis(`$1',`$2')addthis(`$1',`$3')addthis(`$1',`$4')dnl
addthis(`$1',`$5')addthis(`$1',`$6')')dnl

include(`paludis/files.m4')

CLEANFILES = *~
MAINTAINERCLEANFILES = Makefile.in Makefile.am about.hh paludis.hh smart_record.hh comparison_policy.hh
AM_CXXFLAGS = -I$(top_srcdir)
DEFS=-DSYSCONFDIR=\"$(sysconfdir)\"
EXTRA_DIST = about.hh.in Makefile.am.m4 paludis.hh.m4 files.m4 smart_record.hh.m4 comparison_policy.hh.m4 \
        testscriptlist
SUBDIRS = . args

libpaludis_a_SOURCES = filelist

TESTS = testlist

TESTS_ENVIRONMENT = $(SHELL) $(top_srcdir)/test/run_test.sh
check_PROGRAMS = $(TESTS)
check_SCRIPTS = testscriptlist
noinst_LIBRARIES = libpaludis.a

