MAINTAINERCLEANFILES = Makefile.in

libexecprogdir = $(libexecdir)/paludis/utils

if NEED_SED_WRAPPER
libexecprog_SCRIPTS_extra = sed
endif

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
	dounpack \
	emake \
	fowners \
	fperms \
	getmtime \
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
	$(libexecprog_SCRIPTS_extra)

AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@

TESTS_ENVIRONMENT = env \
	PALUDIS_EBUILD_DIR="`$(top_srcdir)/ebuild/utils/canonicalise $(top_srcdir)/ebuild/`" \
	TOP_BUILD_DIR="`$(top_srcdir)/ebuild/utils/canonicalise $(top_builddir)/`" \
	PALUDIS_EBUILD_LOG_LEVEL="silent" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	bash $(top_srcdir)/ebuild/utils/run_test.bash

TESTS = dobin_TEST.bash

prep_scripts = prepall prepallstrip prepallman prepallinfo prepstrip prepman prepinfo prepdocs prepalldocs

EXTRA_DIST = $(libexecprog_SCRIPTS) $(check_SCRIPTS) run_test.bash \
	newlib.so.in newlib.a.in dolib.so.in dolib.a.in prep.in $(TESTS)

CLEANFILES = *~ newlib.so newlib.a dolib.so dolib.a sed $(prep_scripts)

newlib.a : newlib.a.in
	cat $< > $@

newlib.so : newlib.so.in
	cat $< > $@

dolib.so : dolib.so.in
	cat $< > $@

dolib.a : dolib.a.in
	cat $< > $@

sed : sed.in
	cat $< > $@

$(prep_scripts) : prep.in
	cat $< > $@

check_PROGRAMS =

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

