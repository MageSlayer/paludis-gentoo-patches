MAINTAINERCLEANFILES = Makefile.in
SUBDIRS = . 3 exheres-0 kdebuild-1

AUTOMAKE_OPTIONS = parallel-tests

libexecprogdir = $(libexecdir)/paludis/utils

libexecprog_SCRIPTS = \
	canonicalise \
	dobin \
	doconfd \
	dodir \
	dodoc \
	doenvd \
	doexe \
	dohard \
	dohtml \
	doinfo \
	doinitd \
	doins \
	dolib \
	dolib.a \
	dolib.so \
	doman \
	domo \
	donewins \
	dosbin \
	dosed \
	dosym \
	ecompress \
	ecompressdir \
	emake \
	fowners \
	fperms \
	newbin \
	newconfd \
	newdoc \
	newenvd \
	newexe \
	newinitd \
	newins \
	newlib.a \
	newlib.so \
	newman \
	newsbin \
	prepall \
	prepallstrip \
	prepstrip \
	prepallman \
	prepman \
	prepallinfo \
	prepinfo \
	prepdocs \
	prepalldocs \
	unpack \
	wrapped_ldconfig \
	wrapped_getfsize \
	wrapped_getmtime

libexecbindir = $(libexecdir)/paludis/utils
libexecbin_PROGRAMS = \
	print_exports \
	locked_pipe_command

print_exports_SOURCES = print_exports.cc
locked_pipe_command_SOURCES = locked_pipe_command.cc

AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@

TESTS_ENVIRONMENT = env \
	PALUDIS_EBUILD_DIR="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise \
			   $(top_srcdir)/paludis/repositories/e/ebuild/`" \
	PALUDIS_EBUILD_DIR_FALLBACK="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories/e/ebuild/`" \
	TOP_BUILD_DIR="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/`" \
	PALUDIS_EBUILD_LOG_LEVEL="silent" \
	PALUDIS_DISTRIBUTION="gentoo" \
	PALUDIS_OPTIONS="" \
	PALUDIS_NO_CHOWN="yes" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	PALUDIS_TMPDIR="`pwd`/" \
	bash $(top_srcdir)/paludis/repositories/e/ebuild/utils/run_test.bash

TESTS = \
	wrapped_getfsize_TEST.bash

prep_scripts = prepall prepallstrip prepallman prepallinfo prepstrip prepman prepinfo prepdocs prepalldocs

EXTRA_DIST = $(libexecprog_SCRIPTS) $(check_SCRIPTS) run_test.bash prep.in $(TESTS)

CLEANFILES = *~ $(prep_scripts) *.epicfail

$(prep_scripts) : prep.in
	cat $? > $@

check_PROGRAMS =

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

