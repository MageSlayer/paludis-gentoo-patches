ifdef(`__gnu__',`',`errprint(`This is not GNU m4...
')m4exit(1)') include(`misc/generated-file.txt')

dnl vim: set ft=m4 noet :

include $(top_srcdir)/misc/common-makefile.am

define(`filelist', `')dnl
define(`headerlist', `')dnl
define(`testlist', `')dnl
define(`testscriptlist', `')dnl
define(`selist', `')dnl
define(`secleanlist', `')dnl
define(`seheaderlist', `')dnl
define(`srlist', `')dnl
define(`srcleanlist', `')dnl
define(`srheaderlist', `')dnl
define(`addtest', `define(`testlist', testlist `$1_TEST')dnl
$1_TEST_SOURCES = $1_TEST.cc
$1_TEST_LDADD = \
	test_extras.o \
	$(top_builddir)/test/libtest.a \
	libpaludisutil_@PALUDIS_PC_SLOT@.la
$1_TEST_CXXFLAGS = -I$(top_srcdir) $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@
')dnl
define(`addtestscript', `define(`testscriptlist', testscriptlist `$1_TEST_setup.sh $1_TEST_cleanup.sh')')dnl
define(`addhh', `define(`filelist', filelist `$1.hh')define(`headerlist', headerlist `$1.hh')')dnl
define(`addfwd', `define(`filelist', filelist `$1-fwd.hh')define(`headerlist', headerlist `$1-fwd.hh')')dnl
define(`addhhx', `define(`filelist', filelist `$1.hh')')dnl
define(`addcc', `define(`filelist', filelist `$1.cc')')dnl
define(`addimpl', `define(`filelist', filelist `$1-impl.hh')define(`headerlist', headerlist `$1-impl.hh')')dnl
define(`addsr', `define(`srlist', srlist `$1.sr')dnl
define(`srcleanlist', srcleanlist `$1-sr.hh $1-sr.cc')dnl
define(`srheaderlist', srheaderlist `$1-sr.hh')dnl
$1-sr.hh : $1.sr $(top_srcdir)/misc/make_sr.bash
	if ! $(top_srcdir)/misc/make_sr.bash --header $`'(srcdir)/$1.sr > $`'@ ; then rm -f $`'@ ; exit 1 ; fi

$1-sr.cc : $1.sr $(top_srcdir)/misc/make_sr.bash
	if ! $(top_srcdir)/misc/make_sr.bash --source $`'(srcdir)/$1.sr > $`'@ ; then rm -f $`'@ ; exit 1 ; fi

')dnl
define(`addse', `define(`selist', selist `$1.se')dnl
define(`secleanlist', secleanlist `$1-se.hh $1-se.cc')dnl
define(`seheaderlist', seheaderlist `$1-se.hh')dnl
$1-se.hh : $1.se $(top_srcdir)/misc/make_se.bash
	if ! $(top_srcdir)/misc/make_se.bash --header $`'(srcdir)/$1.se > $`'@ ; then rm -f $`'@ ; exit 1 ; fi

$1-se.cc : $1.se $(top_srcdir)/misc/make_se.bash
	if ! $(top_srcdir)/misc/make_se.bash --source $`'(srcdir)/$1.se > $`'@ ; then rm -f $`'@ ; exit 1 ; fi

')dnl
define(`addthis', `dnl
ifelse(`$2', `hh', `addhh(`$1')', `')dnl
ifelse(`$2', `fwd', `addfwd(`$1')', `')dnl
ifelse(`$2', `hhx', `addhhx(`$1')', `')dnl
ifelse(`$2', `cc', `addcc(`$1')', `')dnl
ifelse(`$2', `impl', `addimpl(`$1')', `')dnl
ifelse(`$2', `sr', `addsr(`$1')', `')dnl
ifelse(`$2', `se', `addse(`$1')', `')dnl
ifelse(`$2', `test', `addtest(`$1')', `')dnl
ifelse(`$2', `testscript', `addtestscript(`$1')', `')')dnl
define(`add', `addthis(`$1',`$2')addthis(`$1',`$3')addthis(`$1',`$4')dnl
addthis(`$1',`$5')addthis(`$1',`$6')addthis(`$1',`$7')addthis(`$1',`$8')')dnl

AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_NO_WOLD_STYLE_CAST@ @PALUDIS_CXXFLAGS_VISIBILITY@

include(`paludis/util/files.m4')

MAINTAINERCLEANFILES += Makefile.am paludis.hh \
	hashed_containers.hh util.hh echo_functions.bash
DISTCLEANFILES = srcleanlist secleanlist
BUILT_SOURCES = srcleanlist secleanlist
DEFS=\
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\"
EXTRA_DIST = util.hh.m4 Makefile.am.m4 files.m4 srlist srcleanlist selist secleanlist \
	testscriptlist \
	test_extras.cc \
	echo_functions.bash.in
SUBDIRS = .

libpaludisutil_@PALUDIS_PC_SLOT@_la_SOURCES = filelist
libpaludisutil_@PALUDIS_PC_SLOT@_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0 $(PTHREAD_LIBS) $(RT_LIBS)
libpaludisutil_@PALUDIS_PC_SLOT@_la_LIBADD = $(PTHREAD_LIBS) $(RT_LIBS)

TESTS = testlist

check_PROGRAMS = $(TESTS) system_TEST_become_child
check_SCRIPTS = testscriptlist

system_TEST_become_child_SOURCES = system_TEST_become_child.cc
system_TEST_become_child_LDADD = \
	libpaludisutil_@PALUDIS_PC_SLOT@.la

lib_LTLIBRARIES = libpaludisutil_@PALUDIS_PC_SLOT@.la

paludis_util_includedir = $(includedir)/paludis-$(PALUDIS_PC_SLOT)/paludis/util/
paludis_util_include_HEADERS = headerlist srheaderlist seheaderlist

Makefile.am : Makefile.am.m4 files.m4
	$(top_srcdir)/misc/do_m4.bash Makefile.am

util.hh : util.hh.m4 files.m4
	$(top_srcdir)/misc/do_m4.bash util.hh

libexecutilsdir = $(libexecdir)/paludis/utils
libexecutils_PROGRAMS = outputwrapper

libexecpaludisdir = $(libexecdir)/paludis
libexecpaludis_SCRIPTS = echo_functions.bash

outputwrapper_SOURCES = output_wrapper.cc

