ifdef(`__gnu__',`',`errprint(`This is not GNU m4...
')m4exit(1)') include(`misc/generated-file.txt')

dnl vim: set ft=m4 noet :

define(`filelist', `')dnl
define(`testlist', `')dnl
define(`headerlist', `')dnl
define(`selist', `')dnl
define(`secleanlist', `')dnl
define(`seheaderlist', `')dnl
define(`srlist', `')dnl
define(`srcleanlist', `')dnl
define(`srheaderlist', `')dnl
define(`testscriptlist', `')dnl
define(`addtest', `define(`testlist', testlist `$1_TEST')dnl
$1_TEST_SOURCES = $1_TEST.cc
$1_TEST_LDADD = \
	ihateautomake.o \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/test/libtest.a \
	$(top_builddir)/paludis/environments/test/libpaludistestenvironment.la \
	$(top_builddir)/paludis/repositories/fake/libpaludisfakerepository.la \
	$(top_builddir)/paludis/repositories/virtuals/libpaludisvirtualsrepository.la \
	libpaludis.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(DYNAMIC_LD_LIBS)
$1_TEST_CXXFLAGS = -I$(top_srcdir)
')dnl
define(`addtestscript', `define(`testscriptlist', testscriptlist `$1_TEST_setup.sh $1_TEST_cleanup.sh')')dnl
define(`addhh', `define(`filelist', filelist `$1.hh')define(`headerlist', headerlist `$1.hh')')dnl
define(`addhhx', `define(`filelist', filelist `$1.hh')')dnl
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
define(`addse', `define(`selist', selist `$1.se')dnl
define(`secleanlist', secleanlist `$1-se.hh $1-se.cc')dnl
define(`seheaderlist', seheaderlist `$1-se.hh')dnl
$1-se.hh : $1.se $(top_srcdir)/misc/make_se.bash
	$(top_srcdir)/misc/make_se.bash --header $`'(srcdir)/$1.se > $`'@

$1-se.cc : $1.se $(top_srcdir)/misc/make_se.bash
	$(top_srcdir)/misc/make_se.bash --source $`'(srcdir)/$1.se > $`'@

')dnl
define(`addthis', `dnl
ifelse(`$2', `hh', `addhh(`$1')', `')dnl
ifelse(`$2', `hhx', `addhhx(`$1')', `')dnl
ifelse(`$2', `cc', `addcc(`$1')', `')dnl
ifelse(`$2', `sr', `addsr(`$1')', `')dnl
ifelse(`$2', `se', `addse(`$1')', `')dnl
ifelse(`$2', `impl', `addimpl(`$1')', `')dnl
ifelse(`$2', `test', `addtest(`$1')', `')dnl
ifelse(`$2', `testscript', `addtestscript(`$1')', `')')dnl
define(`add', `addthis(`$1',`$2')addthis(`$1',`$3')addthis(`$1',`$4')dnl
addthis(`$1',`$5')addthis(`$1',`$6')addthis(`$1',`$7')addthis(`$1',`$8')')dnl

include(`paludis/files.m4')

CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda ihateautomake.cc ihateautomake.o
MAINTAINERCLEANFILES = Makefile.in Makefile.am about.hh paludis.hh \
	hashed_containers.hh
DISTCLEANFILES = srcleanlist secleanlist
AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBDIR=\"$(libdir)\"
EXTRA_DIST = about.hh.in Makefile.am.m4 paludis.hh.m4 files.m4 \
	hashed_containers.hh.in testscriptlist srlist srcleanlist selist secleanlist \
	repository_blacklist.txt hooker.bash
SUBDIRS = digests fetchers syncers util selinux . dep_list merger repositories environments args qa tasks
BUILT_SOURCES = srcleanlist secleanlist

libpaludis_la_SOURCES = filelist
libpaludis_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0

libpaludismanpagethings_la_SOURCES = name.cc

if ! MONOLITHIC

libpaludis_la_LIBADD = \
	$(top_builddir)/paludis/util/libpaludisutil.la

libpaludismanpagethings_la_LIBADD = \
	$(top_builddir)/paludis/util/libpaludisutil.la

endif

TESTS = testlist

TESTS_ENVIRONMENT = env \
	PALUDIS_EBUILD_DIR="$(top_srcdir)/paludis/repositories/gentoo/ebuild/" \
	PALUDIS_HOOKER_DIR="$(top_srcdir)/paludis/" \
	PALUDIS_SKIP_CONFIG="yes" \
	PALUDIS_REPOSITORY_SO_DIR="$(top_builddir)/paludis/repositories" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	bash $(top_srcdir)/test/run_test.sh

check_PROGRAMS = $(TESTS)
check_SCRIPTS = testscriptlist

paludis_datadir = $(datadir)/paludis
paludis_data_DATA = repository_blacklist.txt

paludis_libexecdir = $(libexecdir)/paludis
paludis_libexec_SCRIPTS = hooker.bash

if MONOLITHIC

noinst_LTLIBRARIES = libpaludis.la libpaludismanpagethings.la

else

lib_LTLIBRARIES = libpaludis.la
noinst_LTLIBRARIES = libpaludismanpagethings.la

endif

paludis_includedir = $(includedir)/paludis/
paludis_include_HEADERS = headerlist srheaderlist seheaderlist

Makefile.am : Makefile.am.m4 files.m4
	$(top_srcdir)/misc/do_m4.bash Makefile.am

paludis.hh : paludis.hh.m4 files.m4
	$(top_srcdir)/misc/do_m4.bash paludis.hh

comparison_policy.hh : comparison_policy.hh.m4
	$(top_srcdir)/misc/do_m4.bash comparison_policy.hh.m4

ihateautomake.cc : all
	test -f $@ || touch $@

changequote(`<', `>')
built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done


