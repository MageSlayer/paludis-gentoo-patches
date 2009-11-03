include $(top_srcdir)/misc/common-makefile.am

SUBDIRS = .

libexecprog3dir = $(libexecdir)/paludis/utils/3/

libexecprog3_SCRIPTS = \
	docompress \
	dohard \
	dosed \
	banned_in_eapi_3

TESTS =
EXTRA_DIST = \
	$(libexecprog3_SCRIPTS) \
	$(TESTS)

bannedscripts = dohard dosed

$(bannedscripts) : banned_in_eapi_3
	cat $? > $@
	chmod +x $@

CLEANFILES += $(bannedscripts)

