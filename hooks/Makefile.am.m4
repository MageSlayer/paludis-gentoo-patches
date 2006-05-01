ifdef(`__gnu__',`',`errprint(`This is not GNU m4...
')m4exit(1)') include(`misc/generated-file.txt')

dnl vim: set ft=m4 noet :

MAINTAINERCLEANFILES = Makefile.in Makefile.am
CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda .keep
SUBDIRS = .

installhookcommonprogdir = $(libexecdir)/paludis/hooks/common
installhookinstallallpostdir = $(libexecdir)/paludis/hooks/install_all_post

installhookcommonprog_SCRIPTS = \
	gnu_info_index.bash \
	eselect_env_update.bash \
	log.bash

installhookinstallallpost_SCRIPTS = \
	find_config_updates.bash

TESTS_ENVIRONMENT = env \
	PALUDIS_EBUILD_DIR="$(srcdir)/ebuild/" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	$(SHELL) $(top_srcdir)/ebuild/run_test.bash

TESTS =
EXTRA_DIST = \
	Makefile.am.m4 \
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
userhook(`fetch_all_pre')
userhook(`fetch_all_post')
userhook(`fetch_all_all_pre')
userhook(`fetch_all_all_post')
userhook(`ebuild_metadata_pre')
userhook(`ebuild_metadata_post')
userhook(`ebuild_init_pre')
userhook(`ebuild_init_post')
userhook(`ebuild_fetch_pre')
userhook(`ebuild_fetch_post')
userhook(`ebuild_merge_pre')
userhook(`ebuild_merge_post')
userhook(`ebuild_unmerge_pre')
userhook(`ebuild_unmerge_post')
userhook(`ebuild_tidyup_pre')
userhook(`ebuild_tidyup_post')
userhook(`ebuild_strip_pre')
userhook(`ebuild_strip_post')
userhook(`ebuild_unpack_pre')
userhook(`ebuild_unpack_post')
userhook(`ebuild_compile_pre')
userhook(`ebuild_compile_post')
userhook(`ebuild_install_pre')
userhook(`ebuild_install_post')
userhook(`ebuild_test_pre')
userhook(`ebuild_test_post')
userhook(`ebuild_setup_pre')
userhook(`ebuild_setup_post')
userhook(`ebuild_config_pre')
userhook(`ebuild_config_post')
userhook(`ebuild_nofetch_pre')
userhook(`ebuild_nofetch_post')
userhook(`ebuild_preinst_pre')
userhook(`ebuild_preinst_post')
userhook(`ebuild_postinst_pre')
userhook(`ebuild_postinst_post')
userhook(`ebuild_prerm_pre')
userhook(`ebuild_prerm_post')
userhook(`ebuild_postrm_pre')
userhook(`ebuild_postrm_post')

define(`systemhook', `
installsystemhooks$1dir = $(libexecdir)/paludis/hooks/$1
installsystemhooks$1_SCRIPTS = .keep')

systemhook(`install_pre')
systemhook(`install_post')
systemhook(`install_all_pre')
systemhook(`install_all_post')
systemhook(`uninstall_pre')
systemhook(`uninstall_post')
systemhook(`uninstall_all_pre')
systemhook(`uninstall_all_post')
systemhook(`sync_pre')
systemhook(`sync_post')
systemhook(`fetch_all_pre')
systemhook(`fetch_all_post')
systemhook(`fetch_all_all_pre')
systemhook(`fetch_all_all_post')

install-data-local :
	install -d $(DESTDIR)/$(libexecdir)/paludis/hooks/uninstall_all_pre/
	install -d $(DESTDIR)/$(libexecdir)/paludis/hooks/uninstall_all_post/
	install -d $(DESTDIR)/$(libexecdir)/paludis/hooks/uninstall_pre/
	install -d $(DESTDIR)/$(libexecdir)/paludis/hooks/uninstall_post/
	install -d $(DESTDIR)/$(libexecdir)/paludis/hooks/install_pre/
	install -d $(DESTDIR)/$(libexecdir)/paludis/hooks/install_post/
	install -d $(DESTDIR)/$(libexecdir)/paludis/hooks/install_all_pre/
	install -d $(DESTDIR)/$(libexecdir)/paludis/hooks/install_all_post/
	install -d $(DESTDIR)/$(libexecdir)/paludis/hooks/sync_pre/
	install -d $(DESTDIR)/$(libexecdir)/paludis/hooks/sync_post/
	install -d $(DESTDIR)/$(libexecdir)/paludis/hooks/fetch_all_pre/
	install -d $(DESTDIR)/$(libexecdir)/paludis/hooks/fetch_all_post/
	install -d $(DESTDIR)/$(libexecdir)/paludis/hooks/fetch_pre/
	install -d $(DESTDIR)/$(libexecdir)/paludis/hooks/fetch_post/
	ln -sf ../common/gnu_info_index.bash $(DESTDIR)/$(libexecdir)/paludis/hooks/uninstall_all_post/
	ln -sf ../common/gnu_info_index.bash $(DESTDIR)/$(libexecdir)/paludis/hooks/install_all_post/
	ln -sf ../common/eselect_env_update.bash $(DESTDIR)/$(libexecdir)/paludis/hooks/uninstall_all_post/
	ln -sf ../common/eselect_env_update.bash $(DESTDIR)/$(libexecdir)/paludis/hooks/install_post/
	ln -sf ../common/log.bash $(DESTDIR)/$(libexecdir)/paludis/hooks/install_all_pre/
	ln -sf ../common/log.bash $(DESTDIR)/$(libexecdir)/paludis/hooks/install_all_post/
	ln -sf ../common/log.bash $(DESTDIR)/$(libexecdir)/paludis/hooks/install_pre/
	ln -sf ../common/log.bash $(DESTDIR)/$(libexecdir)/paludis/hooks/install_post/
	ln -sf ../common/log.bash $(DESTDIR)/$(libexecdir)/paludis/hooks/fetch_all_pre/
	ln -sf ../common/log.bash $(DESTDIR)/$(libexecdir)/paludis/hooks/fetch_all_post/
	ln -sf ../common/log.bash $(DESTDIR)/$(libexecdir)/paludis/hooks/fetch_pre/
	ln -sf ../common/log.bash $(DESTDIR)/$(libexecdir)/paludis/hooks/fetch_post/
	ln -sf ../common/log.bash $(DESTDIR)/$(libexecdir)/paludis/hooks/uninstall_all_pre/
	ln -sf ../common/log.bash $(DESTDIR)/$(libexecdir)/paludis/hooks/uninstall_all_post/
	ln -sf ../common/log.bash $(DESTDIR)/$(libexecdir)/paludis/hooks/uninstall_pre/
	ln -sf ../common/log.bash $(DESTDIR)/$(libexecdir)/paludis/hooks/uninstall_post/
	ln -sf ../common/log.bash $(DESTDIR)/$(libexecdir)/paludis/hooks/sync_pre/
	ln -sf ../common/log.bash $(DESTDIR)/$(libexecdir)/paludis/hooks/sync_post/

uninstall-local :
	rm $(DESTDIR)/$(libexecdir)/paludis/hooks/*/gnu_info_index.bash
	rm $(DESTDIR)/$(libexecdir)/paludis/hooks/*/eselect_env_update.bash
	rm $(DESTDIR)/$(libexecdir)/paludis/hooks/*/log.bash

Makefile.am : Makefile.am.m4
	$(top_srcdir)/misc/do_m4.bash Makefile.am

