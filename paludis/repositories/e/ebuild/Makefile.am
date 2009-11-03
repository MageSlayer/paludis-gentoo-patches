include $(top_srcdir)/misc/common-makefile.am

SUBDIRS = . 0 1 2 3 exheres-0 kdebuild-1 paludis-1 pbin-1 utils

AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@

libexecprogdir = $(libexecdir)/paludis/

libexecprog_SCRIPTS = \
	binary_functions.bash \
	die_functions.bash \
	ebuild.bash \
	kernel_functions.bash \
	install_functions.bash \
	multilib_functions.bash \
	output_functions.bash \
	pipe_functions.bash \
	sandbox.bash \
	sydbox.bash \
	source_functions.bash \
	usage_error.bash \
	write_vdb_entry.bash \
	write_binary_ebuild.bash

LOG_COMPILER_EXTRA = \
	PALUDIS_EBUILD_LOG_LEVEL="warning" \
	PALUDIS_EBUILD_MODULE_SUFFIXES="0"

LOG_COMPILER_INTERP = $(top_srcdir)/paludis/repositories/e/ebuild/run_test.bash

TESTS = kernel_functions_TEST.bash

EXTRA_DIST = $(libexecprog_SCRIPTS) run_test.bash $(TESTS) $(check_SCRIPTS)

