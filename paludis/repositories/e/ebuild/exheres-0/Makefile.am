MAINTAINERCLEANFILES = Makefile.in
CLEANFILES = *~
SUBDIRS = .

libexecprogexheresdir = $(libexecdir)/paludis/exheres-0

libexecprogexheres_SCRIPTS = \
	build_functions.bash \
	builtin_infovars.bash \
	builtin_init.bash \
	builtin_initrm.bash \
	builtin_initmisc.bash \
	builtin_loadenv.bash \
	builtin_metadata.bash \
	builtin_killold.bash \
	builtin_killoldrm.bash \
	builtin_killoldmisc.bash \
	builtin_saveenv.bash \
	builtin_strip.bash \
	builtin_tidyup.bash \
	builtin_tidyuprm.bash \
	builtin_tidyupmisc.bash \
	builtin_variable.bash \
	conditional_functions.bash \
	eclass_functions.bash \
	ever_functions.bash \
	exlib_functions.bash \
	list_functions.bash \
	output_functions.bash \
	pkg_config.bash \
	pkg_info.bash \
	pkg_nofetch.bash \
	pkg_postinst.bash \
	pkg_postrm.bash \
	pkg_preinst.bash \
	pkg_prerm.bash \
	pkg_pretend.bash \
	pkg_setup.bash \
	portage_stubs.bash \
	src_compile.bash \
	src_configure.bash \
	src_install.bash \
	src_test.bash \
	src_prepare.bash \
	src_unpack.bash

TESTS_ENVIRONMENT = env \
	PALUDIS_EBUILD_DIR="$(top_srcdir)/paludis/repositories/e/ebuild/" \
	PALUDIS_EBUILD_DIR_FALLBACK="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories/e/ebuild/`" \
	PALUDIS_EAPIS_DIR="$(top_srcdir)/paludis/eapis/" \
	PALUDIS_EXTRA_EBUILD_MODULES_DIRS="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/util/`" \
	PALUDIS_DISTRIBUTIONS_DIR="$(top_srcdir)/paludis/distributions/" \
	PALUDIS_DISTRIBUTION="gentoo" \
	PALUDIS_EBUILD_LOG_LEVEL="warning" \
	PALUDIS_EBUILD_MODULE_SUFFIXES="exheres-0" \
	TOP_BUILD_DIR="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/`" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	bash $(top_srcdir)/test/run_test.sh $(top_srcdir)/paludis/repositories/e/ebuild/run_test.bash

TESTS = conditional_functions_TEST.bash list_functions_TEST.bash
check_SCRIPTS = $(TESTS)

EXTRA_DIST = $(libexecprogexheres_SCRIPTS) $(check_SCRIPTS)

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

distcheck-deps-local : $(DISTCHECK_DEPS)

distcheck-deps : distcheck-deps-subdirs

distcheck-deps-subdirs :
	for s in $(SUBDIRS) . ; do if test x$$s = x. ; then $(MAKE) distcheck-deps-local || exit 1 ; \
	    else $(MAKE) -C $$s distcheck-deps || exit 1 ; fi ; done

