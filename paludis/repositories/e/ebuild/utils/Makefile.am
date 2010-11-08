include $(top_srcdir)/misc/common-makefile.am

SUBDIRS = . 4 exheres-0

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

if ENABLE_PBINS
libexecbin_PROGRAMS += \
	unpaxinate

unpaxinate_SOURCES = unpaxinate.cc
unpaxinate_CXXFLAGS = $(AM_CXXFLAGS)
unpaxinate_LDADD = -larchive

endif

print_exports_SOURCES = print_exports.cc
locked_pipe_command_SOURCES = locked_pipe_command.cc

AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@

LOG_COMPILER_EXTRA = \
	PALUDIS_EBUILD_LOG_LEVEL="warning" \
	PALUDIS_EBUILD_MODULE_SUFFIXES="0"

LOG_COMPILER_INTERP = $(top_srcdir)/paludis/repositories/e/ebuild/run_test.bash

TESTS = \
	wrapped_getfsize_TEST.bash

prep_scripts = prepall prepallstrip prepallman prepallinfo prepstrip prepman prepinfo prepdocs prepalldocs

EXTRA_DIST = $(libexecprog_SCRIPTS) $(check_SCRIPTS) run_test.bash prep.in $(TESTS)

CLEANFILES += $(prep_scripts)

$(prep_scripts) : prep.in
	cat $? > $@

check_PROGRAMS =

