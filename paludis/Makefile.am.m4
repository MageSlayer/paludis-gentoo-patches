ifdef(`__gnu__',`',`errprint(`This is not GNU m4...
')m4exit(1)') include(`misc/generated-file.txt')

dnl vim: set ft=m4 noet :

include $(top_srcdir)/misc/common-makefile.am

define(`filelist', `')dnl
define(`gtestlist', `')dnl
define(`headerlist', `')dnl
define(`selist', `')dnl
define(`secleanlist', `')dnl
define(`seheaderlist', `')dnl
define(`testscriptlist', `')dnl
define(`addgtest', `define(`gtestlist', gtestlist `$1_TEST')dnl
$1_TEST_SOURCES = $1_TEST.cc
$1_TEST_LDADD = \
	$(top_builddir)/paludis/util/gtest_runner.o \
	libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(DYNAMIC_LD_LIBS)
$1_TEST_LDFLAGS = @GTESTDEPS_LDFLAGS@ @GTESTDEPS_LIBS@
$1_TEST_CXXFLAGS = -I$(top_srcdir) $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@ @GTESTDEPS_CXXFLAGS@
')dnl
define(`addtestscript', `define(`testscriptlist', testscriptlist `$1_TEST_setup.sh $1_TEST_cleanup.sh')')dnl
define(`addhh', `define(`filelist', filelist `$1.hh')define(`headerlist', headerlist `$1.hh')')dnl
define(`addfwd', `define(`filelist', filelist `$1-fwd.hh')define(`headerlist', headerlist `$1-fwd.hh')')dnl
define(`addhhx', `define(`filelist', filelist `$1.hh')')dnl
define(`addcc', `define(`filelist', filelist `$1.cc')')dnl
define(`addimpl', `define(`filelist', filelist `$1-impl.hh')define(`headerlist', headerlist `$1-impl.hh')')dnl
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
ifelse(`$2', `se', `addse(`$1')', `')dnl
ifelse(`$2', `impl', `addimpl(`$1')', `')dnl
ifelse(`$2', `gtest', `addgtest(`$1')', `')dnl
ifelse(`$2', `testscript', `addtestscript(`$1')', `')')dnl
define(`add', `addthis(`$1',`$2')addthis(`$1',`$3')addthis(`$1',`$4')dnl
addthis(`$1',`$5')addthis(`$1',`$6')addthis(`$1',`$7')addthis(`$1',`$8')')dnl

AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@

include(`paludis/files.m4')

MAINTAINERCLEANFILES += Makefile.am about.hh paludis.hh
DISTCLEANFILES = secleanlist
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBDIR=\"$(libdir)\" \
	-DPYTHONINSTALLDIR=\"$(PYTHON_INSTALL_DIR)\"
EXTRA_DIST = about.hh.in Makefile.am.m4 paludis.hh.m4 files.m4 \
	testscriptlist selist secleanlist \
	hooker.bash \
	stripper_TEST_binary.cc
SUBDIRS = distributions fetchers syncers util selinux repositories environments . args resolver
BUILT_SOURCES = secleanlist

libpaludis_@PALUDIS_PC_SLOT@_la_SOURCES = filelist
libpaludis_@PALUDIS_PC_SLOT@_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0 $(PTHREAD_LIBS)

libpaludispythonhooks_@PALUDIS_PC_SLOT@_la_SOURCES = python_hooks.cc
libpaludispythonhooks_@PALUDIS_PC_SLOT@_la_CXXFLAGS = $(AM_CXXFLAGS) \
	@PALUDIS_CXXFLAGS_NO_STRICT_ALIASING@ \
	-I@PYTHON_INCLUDE_DIR@
libpaludispythonhooks_@PALUDIS_PC_SLOT@_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0 @BOOST_PYTHON_LIB@ -lpython@PYTHON_VERSION@
libpaludispythonhooks_@PALUDIS_PC_SLOT@_la_LIBADD = libpaludis_@PALUDIS_PC_SLOT@.la

libpaludissohooks_TEST_@PALUDIS_PC_SLOT@_la_SOURCES = sohooks_TEST.cc

# -rpath to force shared library
libpaludissohooks_TEST_@PALUDIS_PC_SLOT@_la_LDFLAGS = -rpath /nowhere -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0

libpaludissohooks_TEST_@PALUDIS_PC_SLOT@_la_LIBADD = libpaludis_@PALUDIS_PC_SLOT@.la

repositories_libadd =

define(`condrepo', `
if ENABLE_`'translit($1,`a-z',`A-Z')_REPOSITORY
repositories_libadd += $(top_builddir)/paludis/repositories/$1/libpaludis$1repository.la
endif
')

condrepo(accounts)
condrepo(dummy)
condrepo(e)
condrepo(fake)
condrepo(gemcutter)
condrepo(repository)
condrepo(unavailable)
condrepo(unpackaged)
condrepo(unwritten)

environments_libadd =

define(`condenv', `
if ENABLE_`'translit($1,`a-z',`A-Z')_ENVIRONMENT
environments_libadd += $(top_builddir)/paludis/environments/$1/libpaludis`'translit($1,_,)environment.la
endif
')

condenv(dummy)
condenv(paludis)
condenv(portage)
condenv(test)

libpaludis_@PALUDIS_PC_SLOT@_la_LIBADD = \
	$(top_builddir)/paludis/selinux/libpaludisselinux_@PALUDIS_PC_SLOT@.la \
	$(repositories_libadd) \
	$(environments_libadd) \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	@DYNAMIC_LD_LIBS@ \
	$(PTHREAD_LIBS)

if HAVE_GTEST
TESTS = gtestlist
else
TESTS =
endif

check_PROGRAMS = $(TESTS) stripper_TEST_binary
check_SCRIPTS = testscriptlist
check_LTLIBRARIES = libpaludissohooks_TEST_@PALUDIS_PC_SLOT@.la

stripper_TEST_binary_SOURCES = stripper_TEST_binary.cc

paludis_libexecdir = $(libexecdir)/paludis
paludis_libexec_SCRIPTS = hooker.bash

lib_LTLIBRARIES = libpaludis_@PALUDIS_PC_SLOT@.la

if ENABLE_PYTHON_HOOKS
lib_LTLIBRARIES += libpaludispythonhooks_@PALUDIS_PC_SLOT@.la
endif

if ENABLE_PBINS
lib_LTLIBRARIES += libpaludistarextras_@PALUDIS_PC_SLOT@.la
endif

libpaludistarextras_@PALUDIS_PC_SLOT@_la_SOURCES = tar_extras.cc tar_extras.hh
libpaludistarextras_@PALUDIS_PC_SLOT@_la_CXXFLAGS = $(AM_CXXFLAGS)
libpaludistarextras_@PALUDIS_PC_SLOT@_la_LIBADD = -larchive
libpaludistarextras_@PALUDIS_PC_SLOT@_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0

if ENABLE_STRIPPER
lib_LTLIBRARIES += libpaludisstripperextras_@PALUDIS_PC_SLOT@.la
endif

libpaludisstripperextras_@PALUDIS_PC_SLOT@_la_SOURCES = stripper_extras.cc stripper_extras.hh
libpaludisstripperextras_@PALUDIS_PC_SLOT@_la_CXXFLAGS = $(AM_CXXFLAGS) $(LIBMAGICDEPS_CFLAGS)
libpaludisstripperextras_@PALUDIS_PC_SLOT@_la_LIBADD = $(LIBMAGICDEPS_LIBS)
libpaludisstripperextras_@PALUDIS_PC_SLOT@_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0

paludis_includedir = $(includedir)/paludis-$(PALUDIS_PC_SLOT)/paludis/
paludis_include_HEADERS = headerlist seheaderlist

Makefile.am : Makefile.am.m4 files.m4
	$(top_srcdir)/misc/do_m4.bash Makefile.am

paludis.hh : paludis.hh.m4 files.m4
	$(top_srcdir)/misc/do_m4.bash paludis.hh

comparison_policy.hh : comparison_policy.hh.m4
	$(top_srcdir)/misc/do_m4.bash comparison_policy.hh.m4

