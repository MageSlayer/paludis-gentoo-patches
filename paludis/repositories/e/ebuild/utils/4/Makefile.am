include $(top_srcdir)/misc/common-makefile.am

SUBDIRS = .

libexecprog4dir = $(libexecdir)/paludis/utils/4/

libexecprog4_SCRIPTS = \
	docompress \
	dohard \
	dosed \
	banned_in_eapi_4

TESTS =
EXTRA_DIST = \
	$(libexecprog4_SCRIPTS) \
	$(TESTS)

bannedscripts = dohard dosed

$(bannedscripts) : banned_in_eapi_4
	cat $? > $@
	chmod +x $@

CLEANFILES += $(bannedscripts)

