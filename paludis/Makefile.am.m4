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
	libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(DYNAMIC_LD_LIBS)
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
ifelse(`$2', `sr', `addsr(`$1')', `')dnl
ifelse(`$2', `se', `addse(`$1')', `')dnl
ifelse(`$2', `impl', `addimpl(`$1')', `')dnl
ifelse(`$2', `test', `addtest(`$1')', `')dnl
ifelse(`$2', `testscript', `addtestscript(`$1')', `')')dnl
define(`add', `addthis(`$1',`$2')addthis(`$1',`$3')addthis(`$1',`$4')dnl
addthis(`$1',`$5')addthis(`$1',`$6')addthis(`$1',`$7')addthis(`$1',`$8')')dnl

AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@

include(`paludis/files.m4')

CLEANFILES = *~ gmon.out *.gcov *.gcno  *.gcda *.loT ihateautomake.cc ihateautomake.o *.epicfail
MAINTAINERCLEANFILES = Makefile.in Makefile.am about.hh paludis.hh
DISTCLEANFILES = srcleanlist secleanlist
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBDIR=\"$(libdir)\" \
	-DPYTHONINSTALLDIR=\"$(PYTHON_INSTALL_DIR)\"
EXTRA_DIST = about.hh.in Makefile.am.m4 paludis.hh.m4 files.m4 \
	testscriptlist srlist srcleanlist selist secleanlist \
	hooker.bash \
	stripper_TEST_binary.cc
SUBDIRS = distributions fetchers syncers util selinux repositories environments . args resolver
BUILT_SOURCES = srcleanlist secleanlist

libpaludis_@PALUDIS_PC_SLOT@_la_SOURCES = filelist
libpaludis_@PALUDIS_PC_SLOT@_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0 $(PTHREAD_LIBS)

libpaludispythonhooks_@PALUDIS_PC_SLOT@_la_SOURCES = python_hooks.cc
libpaludispythonhooks_@PALUDIS_PC_SLOT@_la_CXXFLAGS = $(AM_CXXFLAGS) \
	@PALUDIS_CXXFLAGS_NO_STRICT_ALIASING@ \
	@PALUDIS_CXXFLAGS_NO_WSHADOW@ \
	-I@PYTHON_INCLUDE_DIR@
libpaludispythonhooks_@PALUDIS_PC_SLOT@_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0 @BOOST_PYTHON_LIB@ -lpython@PYTHON_VERSION@
libpaludispythonhooks_@PALUDIS_PC_SLOT@_la_LIBADD = libpaludis_@PALUDIS_PC_SLOT@.la

libpaludismanpagethings_@PALUDIS_PC_SLOT@_la_SOURCES = name.cc

libpaludissohooks_TEST_@PALUDIS_PC_SLOT@_la_SOURCES = sohooks_TEST.cc

# -rpath to force shared library
libpaludissohooks_TEST_@PALUDIS_PC_SLOT@_la_LDFLAGS = -rpath /nowhere -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0

libpaludissohooks_TEST_@PALUDIS_PC_SLOT@_la_LIBADD = libpaludis_@PALUDIS_PC_SLOT@.la

libpaludis_@PALUDIS_PC_SLOT@_la_LIBADD = \
	$(top_builddir)/paludis/selinux/libpaludisselinux_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/repositories/accounts/libpaludisaccountsrepository.la \
	$(top_builddir)/paludis/repositories/cran/libpaludiscranrepository.la \
	$(top_builddir)/paludis/repositories/e/libpaludiserepository.la \
	$(top_builddir)/paludis/repositories/fake/libpaludisfakerepository.la \
	$(top_builddir)/paludis/repositories/gems/libpaludisgemsrepository.la \
	$(top_builddir)/paludis/repositories/unavailable/libpaludisunavailablerepository.la \
	$(top_builddir)/paludis/repositories/unpackaged/libpaludisunpackagedrepository.la \
	$(top_builddir)/paludis/repositories/unwritten/libpaludisunwrittenrepository.la \
	$(top_builddir)/paludis/repositories/virtuals/libpaludisvirtualsrepository.la \
	$(top_builddir)/paludis/environments/no_config/libpaludisnoconfigenvironment.la \
	$(top_builddir)/paludis/environments/paludis/libpaludispaludisenvironment.la \
	$(top_builddir)/paludis/environments/portage/libpaludisportageenvironment.la \
	$(top_builddir)/paludis/environments/test/libpaludistestenvironment.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	@DYNAMIC_LD_LIBS@ \
	$(PTHREAD_LIBS)

libpaludismanpagethings_@PALUDIS_PC_SLOT@_la_LIBADD = \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la

dep_list_TEST_SOURCES += dep_list_TEST.hh
define(`testlist', testlist `dep_list_TEST_blockers')dnl
dep_list_TEST_blockers_SOURCES = dep_list_TEST_blockers.cc dep_list_TEST.hh
dep_list_TEST_blockers_LDADD = \
	ihateautomake.o \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/test/libtest.a \
	libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(DYNAMIC_LD_LIBS)
dep_list_TEST_blockers_CXXFLAGS = -I$(top_srcdir) $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

TESTS = testlist

check_PROGRAMS = $(TESTS) stripper_TEST_binary
check_SCRIPTS = testscriptlist
check_LTLIBRARIES = libpaludissohooks_TEST_@PALUDIS_PC_SLOT@.la

stripper_TEST_binary_SOURCES = stripper_TEST_binary.cc

paludis_libexecdir = $(libexecdir)/paludis
paludis_libexec_SCRIPTS = hooker.bash

if MONOLITHIC

noinst_LTLIBRARIES = libpaludis_@PALUDIS_PC_SLOT@.la libpaludismanpagethings_@PALUDIS_PC_SLOT@.la

else

lib_LTLIBRARIES = libpaludis_@PALUDIS_PC_SLOT@.la
noinst_LTLIBRARIES = libpaludismanpagethings_@PALUDIS_PC_SLOT@.la

if ENABLE_PYTHON_HOOKS
lib_LTLIBRARIES += libpaludispythonhooks_@PALUDIS_PC_SLOT@.la
endif

endif


paludis_includedir = $(includedir)/paludis-$(PALUDIS_PC_SLOT)/paludis/
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

DISTCHECK_DEPS = libpaludis_@PALUDIS_PC_SLOT@.la libpaludismanpagethings_@PALUDIS_PC_SLOT@.la

distcheck-deps-local : $(DISTCHECK_DEPS)

distcheck-deps : distcheck-deps-subdirs

distcheck-deps-subdirs :
	for s in $(SUBDIRS) . ; do if test x$$s = x. ; then $(MAKE) distcheck-deps-local || exit 1 ; \
	    else $(MAKE) -C $$s distcheck-deps || exit 1 ; fi ; done

TESTS_ENVIRONMENT = env \
	PALUDIS_EBUILD_DIR="$(top_srcdir)/paludis/repositories/e/ebuild/" \
	PALUDIS_EAPIS_DIR="$(top_srcdir)/paludis/repositories/e/eapis/" \
	PALUDIS_DISTRIBUTIONS_DIR="$(top_srcdir)/paludis/distributions/" \
	PALUDIS_DISTRIBUTION="gentoo" \
	PALUDIS_HOOKER_DIR="$(top_srcdir)/paludis/" \
	PALUDIS_OPTIONS="" \
	PALUDIS_OUTPUTWRAPPER_DIR="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/util/`" \
	PALUDIS_SKIP_CONFIG="yes" \
	PALUDIS_REPOSITORY_SO_DIR="$(top_builddir)/paludis/repositories" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	SO_SUFFIX=@VERSION_LIB_CURRENT@ \
	PALUDIS_PC_SLOT=@PALUDIS_PC_SLOT@ \
	PYTHONPATH="$(top_builddir)/python/" \
	PALUDIS_PYTHON_DIR="$(top_srcdir)/python/" \
	PALUDIS_DEFAULT_OUTPUT_CONF="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_srcdir)/paludis/environments/paludis/tests_output.conf`" \
	PALUDIS_OUTPUT_MANAGERS_DIR="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_srcdir)/paludis/environments/paludis/output_managers/`" \
	LD_LIBRARY_PATH="`echo $$LD_LIBRARY_PATH: | sed -e 's,^:,,'`` \
		$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/.libs/ \
		`:`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/util/.libs/ \
		`:`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/python/.libs/`" \
	bash $(top_srcdir)/test/run_test.sh

