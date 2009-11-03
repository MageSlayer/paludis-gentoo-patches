include $(top_srcdir)/misc/common-makefile.am

SUBDIRS = .

libexecprog2dir = $(libexecdir)/paludis/2

libexecprog2_SCRIPTS = \
	eclass_functions.bash \
	pkg_nofetch.bash \
	src_compile.bash \
	src_configure.bash \
	src_install.bash \
	src_prepare.bash \
	src_test.bash \
	src_unpack.bash \
	output_functions.bash

TESTS =
check_SCRIPTS = $(TESTS)

EXTRA_DIST = $(libexecprog2_SCRIPTS)

