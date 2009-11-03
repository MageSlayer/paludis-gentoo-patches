include $(top_srcdir)/misc/common-makefile.am

SUBDIRS = .

libexecprogdir = $(libexecdir)/paludis/utils/kdebuild-1

libexecprog_SCRIPTS = \
	dohard \
	dohtml \
	dosed \
	donewins \
	prepall \
	prepallstrip \
	prepstrip \
	prepallman \
	prepman \
	prepallinfo \
	prepinfo \
	prepdocs \
	prepalldocs \
	ecompress \
	ecompressdir \
	banned_in_eapi_kdebuild-1

AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@

TESTS =

EXTRA_DIST = $(libexecprog_SCRIPTS) $(check_SCRIPTS) \
	banned_in_eapi_kdebuild-1 $(TESTS)

bannedscripts = prepall prepallstrip prepstrip prepallman prepman prepallinfo \
		prepinfo prepdocs prepalldocs dohard donewins dosed dohtml \
		ecompress ecompressdir

CLEANFILES += $(bannedscripts)

$(bannedscripts) : banned_in_eapi_kdebuild-1
	cat $? > $@
	chmod +x $@

check_PROGRAMS =

