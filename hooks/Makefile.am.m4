ifdef(`__gnu__',`',`errprint(`This is not GNU m4...
')m4exit(1)') include(`misc/generated-file.txt')

dnl vim: set ft=m4 noet :

MAINTAINERCLEANFILES = Makefile.in
CLEANFILES = *~ .keep
SUBDIRS = .

installhookcommonprogdir = $(libexecdir)/paludis/hooks/common
installhookinstallallpostdir = $(libexecdir)/paludis/hooks/install_all_post

installhookcommonprog_SCRIPTS = \
	gnu_info_index.bash \
	eselect_env_update.bash

installhookinstallallpost_SCRIPTS = \
	find_config_updates.bash

TESTS_ENVIRONMENT = env \
	PALUDIS_EBUILD_DIR="$(srcdir)/ebuild/" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	$(SHELL) $(top_srcdir)/ebuild/run_test.bash

TESTS =
EXTRA_DIST = \
	$(installhookcommonprog_SCRIPTS) \
	$(installhookinstallallpost_SCRIPTS) \
	$(TESTS)

check_SCRIPTS = $(TESTS)
check_PROGRAMS =

.keep :
	touch $@

define(`userhook', `
installuserhooks$1dir = $(datadir)/paludis/hooks/$1
installuserhooks$1_SCRIPTS = .keep')

userhook(`install_pre')
userhook(`install_post')
userhook(`install_all_pre')
userhook(`install_all_post')
userhook(`uninstall_pre')
userhook(`uninstall_post')
userhook(`uninstall_all_pre')
userhook(`uninstall_all_post')
userhook(`sync_pre')
userhook(`sync_post')
userhook(`sync_all_pre')
userhook(`sync_all_post')

define(`systemhook', `
installsystemhooks$1dir = $(libexecdir)/paludis/hooks/$1
installsystemhooks$1_SCRIPTS = .keep')

systemhook(`install_post')
systemhook(`install_all_post')
systemhook(`uninstall_all_post')

install-data-local :
	install -d $(DESTDIR)/$(libexecdir)/paludis/hooks/uninstall_all_post/
	install -d $(DESTDIR)/$(libexecdir)/paludis/hooks/install_all_post/
	install -d $(DESTDIR)/$(libexecdir)/paludis/hooks/install_post/
	ln -sf ../common/gnu_info_index.bash $(DESTDIR)/$(libexecdir)/paludis/hooks/uninstall_all_post/
	ln -sf ../common/gnu_info_index.bash $(DESTDIR)/$(libexecdir)/paludis/hooks/install_all_post/
	ln -sf ../common/eselect_env_update.bash $(DESTDIR)/$(libexecdir)/paludis/hooks/uninstall_all_post/
	ln -sf ../common/eselect_env_update.bash $(DESTDIR)/$(libexecdir)/paludis/hooks/install_post/

uninstall-local :
	rm $(DESTDIR)/$(libexecdir)/paludis/hooks/uninstall_all_post/gnu_info_index.bash
	rm $(DESTDIR)/$(libexecdir)/paludis/hooks/install_all_post/gnu_info_index.bash
	rm $(DESTDIR)/$(libexecdir)/paludis/hooks/uninstall_all_post/eselect_env_update.bash
	rm $(DESTDIR)/$(libexecdir)/paludis/hooks/install_post/eselect_env_update.bash

Makefile.am : Makefile.am.m4
	$(top_srcdir)/misc/do_m4.bash Makefile.am

