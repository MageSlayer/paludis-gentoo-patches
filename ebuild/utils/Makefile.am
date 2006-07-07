MAINTAINERCLEANFILES = Makefile.in
CLEANFILES = *~ newlib.so newlib.a dolib.so dolib.a sed

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
	$(libexecprog_SCRIPTS_extra)

libexecprog_PROGRAMS = merge unmerge
AM_CXXFLAGS = -I$(top_srcdir)

merge_SOURCES = merge.cc merge_common.cc merge_common.hh
merge_LDADD = \
	$(top_builddir)/paludis/util/libpaludisutil.a \
	$(top_builddir)/paludis/digests/libpaludisdigests.a \
	$(top_builddir)/paludis/selinux/libpaludisselinux.a \
	$(DYNAMIC_LD_LIBS)

unmerge_SOURCES = unmerge.cc merge_common.cc merge_common.hh
unmerge_LDADD = \
	$(top_builddir)/paludis/util/libpaludisutil.a \
	$(top_builddir)/paludis/digests/libpaludisdigests.a

TESTS_ENVIRONMENT = env \
	PALUDIS_EBUILD_DIR="`$(top_srcdir)/ebuild/utils/canonicalise $(top_srcdir)/ebuild/`" \
	TOP_BUILD_DIR="`$(top_srcdir)/ebuild/utils/canonicalise $(top_builddir)/`" \
	PALUDIS_EBUILD_LOG_LEVEL="silent" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	bash $(top_srcdir)/ebuild/utils/run_test.bash

TESTS = merge_TEST.bash dobin_TEST.bash unmerge_TEST.bash
check_SCRIPTS = $(TESTS) \
	merge_TEST_setup.sh merge_TEST_cleanup.sh \
	unmerge_TEST_setup.sh unmerge_TEST_cleanup.sh

EXTRA_DIST = $(libexecprog_SCRIPTS) $(check_SCRIPTS) run_test.bash \
	newlib.so.in newlib.a.in dolib.so.in dolib.a.in prep.in

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

prepall: prep.in
	cat $< >$@

prepallstrip: prep.in
	cat $< >$@

prepstrip: prep.in
	cat $< >$@

prepallman: prep.in
	cat $< >$@

prepman: prep.in
	cat $< >$@

prepallinfo: prep.in
	cat $< >$@

prepinfo: prep.in
	cat $< >$@

check_PROGRAMS =

