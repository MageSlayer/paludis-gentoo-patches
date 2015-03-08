include $(top_srcdir)/misc/common-makefile.am

SUBDIRS = .

libexecprogdir = $(libexecdir)/paludis/utils/exheres-0

libexecprog_SCRIPTS = \
	dohard \
	dohtml \
	dolib \
	dosbin \
	dosed \
	donewins \
	emake \
	fperms \
	fowners \
	herebin \
	hereconfd \
	hereenvd \
	hereinitd \
	hereins \
	heresbin \
	newsbin \
	nonfatal \
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
	strip \
	banned_in_eapi_exheres-0

AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@

TESTS =

EXTRA_DIST = \
	$(check_SCRIPTS) \
	$(libexecprog_SCRIPTS) \
	$(TESTS)

bannedscripts = prepall prepallstrip prepstrip prepallman prepman prepallinfo \
		prepinfo prepdocs prepalldocs dohard donewins dosed dohtml \
		ecompress ecompressdir fperms fowners

CLEANFILES += $(bannedscripts)

$(bannedscripts) : banned_in_eapi_exheres-0
	cat $? > $@
	chmod +x $@

check_PROGRAMS =

