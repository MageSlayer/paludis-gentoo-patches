include $(top_srcdir)/misc/common-makefile.am

SUBDIRS = .

libexecprog5dir = $(libexecdir)/paludis/5

libexecprog5_SCRIPTS = \
	src_prepare.bash \
	src_test.bash \
	apply_user_patches.bash \
	usex.bash \
	output_functions.bash

TESTS =
check_SCRIPTS = $(TESTS)

EXTRA_DIST = $(libexecprog5_SCRIPTS)

