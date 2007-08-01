MAINTAINERCLEANFILES = Makefile.in
SUBDIRS = . exheres-0

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
	emake \
	fowners \
	fperms \
	getmtime \
	getfsize \
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
	unpack

AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@

TESTS_ENVIRONMENT = env \
	PALUDIS_EBUILD_DIR="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise \
			   $(top_srcdir)/paludis/repositories/e/ebuild/`" \
	TOP_BUILD_DIR="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/`" \
	PALUDIS_EBUILD_LOG_LEVEL="silent" \
	PALUDIS_DISTRIBUTION="gentoo" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	bash $(top_srcdir)/paludis/repositories/e/ebuild/utils/run_test.bash

TESTS = dobin_TEST.bash getfsize_TEST.bash

prep_scripts = prepall prepallstrip prepallman prepallinfo prepstrip prepman prepinfo prepdocs prepalldocs

EXTRA_DIST = $(libexecprog_SCRIPTS) $(check_SCRIPTS) run_test.bash \
	newlib.so.in newlib.a.in dolib.so.in dolib.a.in prep.in $(TESTS)

CLEANFILES = *~ newlib.so newlib.a dolib.so dolib.a $(prep_scripts)

newlib.a : newlib.a.in
	cat $? > $@

newlib.so : newlib.so.in
	cat $? > $@

dolib.so : dolib.so.in
	cat $? > $@

dolib.a : dolib.a.in
	cat $? > $@

$(prep_scripts) : prep.in
	cat $? > $@

check_PROGRAMS =

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

