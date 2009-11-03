ifdef(`__gnu__',`',`errprint(`This is not GNU m4...
')m4exit(1)') include(`misc/generated-file.txt')

dnl vim: set ft=m4 noet :

include $(top_srcdir)/misc/common-makefile.am

CLEANFILES += .keep
MAINTAINERCLEANFILES += Makefile.am
SUBDIRS = . demos

installvarlibpaludisnewsdir = $(localstatedir)/gentoo/news

installhookcommonprogdir = $(libexecdir)/paludis/hooks/common
installhookautoprogdir = $(libexecdir)/paludis/hooks/auto
installhookinstallpostdir = $(libexecdir)/paludis/hooks/install_post

installhookcommonprog_SCRIPTS = \
	gnu_info_index.bash \
	eselect_env_update.bash \
	log.bash \
	installable_cache_regen.bash \
	installed_cache_regen.bash

installhookautoprog_SCRIPTS = \
	news.hook \
	find_config_updates.hook

installhookinstallpost_SCRIPTS = \
	update_config_protect_list.bash

installvarlibpaludisnews_DATA = \
	.keep

TESTS = \
	eselect_env_update_TEST \
	news_TEST

check_SCRIPTS = $(TESTS) run_test.bash \
	eselect_env_update_TEST_setup.sh eselect_env_update_TEST_cleanup.sh \
	news_TEST_setup.sh news_TEST_cleanup.sh

check_PROGRAMS =

EXTRA_DIST = \
	Makefile.am.m4 \
	gnu_info_index.bash \
	eselect_env_update.bash.in \
	log.bash \
	installable_cache_regen.bash \
	installed_cache_regen.bash \
	news.hook.in \
	find_config_updates.hook \
	$(installhookinstallpost_SCRIPTS) \
	$(check_SCRIPTS)

.keep :
	touch $@

define(`userhook', `
installuserhooks$1dir = $(datadir)/paludis/hooks/$1
installuserhooks$1_DATA = .keep
installuserhooksplat$1dir = $(libdir)/paludis/hooks/$1
installuserhooksplat$1_DATA = .keep')

userhook(`auto')
userhook(`install_pre')
userhook(`install_fail')
userhook(`install_post')
userhook(`install_all_pre')
userhook(`install_all_post')
userhook(`install_pretend_pre')
userhook(`install_pretend_post')
userhook(`install_pretend_display_item_pre')
userhook(`install_pretend_display_item_post')
userhook(`install_task_execute_pre')
userhook(`install_task_execute_post')
userhook(`clean_all_post')
userhook(`clean_all_pre')
userhook(`clean_fail')
userhook(`clean_post')
userhook(`clean_pre')
userhook(`uninstall_pre')
userhook(`uninstall_fail')
userhook(`uninstall_post')
userhook(`uninstall_all_pre')
userhook(`uninstall_all_post')
userhook(`sync_pre')
userhook(`sync_fail')
userhook(`sync_post')
userhook(`sync_all_pre')
userhook(`sync_all_post')
userhook(`fetch_pre')
userhook(`fetch_post')
userhook(`fetch_all_pre')
userhook(`fetch_all_post')

userhook(`ebuild_metadata_pre')
userhook(`ebuild_metadata_fail')
userhook(`ebuild_metadata_post')
userhook(`ebuild_init_pre')
userhook(`ebuild_init_fail')
userhook(`ebuild_init_post')
userhook(`ebuild_fetch_pre')
userhook(`ebuild_fetch_fail')
userhook(`ebuild_fetch_post')
userhook(`ebuild_tidyup_pre')
userhook(`ebuild_tidyup_fail')
userhook(`ebuild_tidyup_post')
userhook(`ebuild_strip_pre')
userhook(`ebuild_strip_fail')
userhook(`ebuild_strip_post')
userhook(`ebuild_unpack_pre')
userhook(`ebuild_unpack_fail')
userhook(`ebuild_unpack_post')
userhook(`ebuild_prepare_pre')
userhook(`ebuild_prepare_fail')
userhook(`ebuild_prepare_post')
userhook(`ebuild_configure_pre')
userhook(`ebuild_configure_fail')
userhook(`ebuild_configure_post')
userhook(`ebuild_compile_pre')
userhook(`ebuild_compile_fail')
userhook(`ebuild_compile_post')
userhook(`ebuild_install_pre')
userhook(`ebuild_install_fail')
userhook(`ebuild_install_post')
userhook(`ebuild_test_pre')
userhook(`ebuild_test_fail')
userhook(`ebuild_test_post')
userhook(`ebuild_setup_pre')
userhook(`ebuild_setup_fail')
userhook(`ebuild_setup_post')
userhook(`ebuild_config_pre')
userhook(`ebuild_config_fail')
userhook(`ebuild_config_post')
userhook(`ebuild_nofetch_pre')
userhook(`ebuild_nofetch_fail')
userhook(`ebuild_nofetch_post')
userhook(`ebuild_preinst_pre')
userhook(`ebuild_preinst_fail')
userhook(`ebuild_preinst_post')
userhook(`ebuild_postinst_pre')
userhook(`ebuild_postinst_fail')
userhook(`ebuild_postinst_post')
userhook(`ebuild_prerm_pre')
userhook(`ebuild_prerm_fail')
userhook(`ebuild_prerm_post')
userhook(`ebuild_postrm_pre')
userhook(`ebuild_postrm_fail')
userhook(`ebuild_postrm_post')

userhook(`einfo')
userhook(`ewarn')
userhook(`eerror')
userhook(`elog')

userhook(`merger_unlink_file_pre')
userhook(`merger_unlink_file_post')
userhook(`merger_unlink_dir_pre')
userhook(`merger_unlink_dir_post')
userhook(`merger_unlink_sym_pre')
userhook(`merger_unlink_sym_post')
userhook(`merger_unlink_misc_pre')
userhook(`merger_unlink_misc_post')
userhook(`merger_install_pre')
userhook(`merger_install_post')
userhook(`merger_install_file_pre')
userhook(`merger_install_file_post')
userhook(`merger_install_sym_pre')
userhook(`merger_install_sym_post')
userhook(`merger_install_dir_pre')
userhook(`merger_install_dir_post')
userhook(`merger_check_pre')
userhook(`merger_check_post')
userhook(`merger_check_file_pre')
userhook(`merger_check_file_post')
userhook(`merger_check_sym_pre')
userhook(`merger_check_sym_post')
userhook(`merger_check_dir_pre')
userhook(`merger_check_dir_post')
userhook(`unmerger_unlink_pre')
userhook(`unmerger_unlink_post')
userhook(`unmerger_unlink_file_pre')
userhook(`unmerger_unlink_file_post')
userhook(`unmerger_unlink_dir_pre')
userhook(`unmerger_unlink_dir_post')
userhook(`unmerger_unlink_sym_pre')
userhook(`unmerger_unlink_sym_post')
userhook(`unmerger_unlink_misc_pre')
userhook(`unmerger_unlink_misc_post')

define(`systemhook', `
installsystemhooks$1dir = $(libexecdir)/paludis/hooks/$1
installsystemhooks$1_DATA = .keep')

systemhook(`auto')
systemhook(`install_pre')
systemhook(`install_post')
systemhook(`install_all_pre')
systemhook(`install_all_post')
systemhook(`install_pretend_pre')
systemhook(`install_pretend_post')
systemhook(`uninstall_pre')
systemhook(`uninstall_post')
systemhook(`uninstall_all_pre')
systemhook(`uninstall_all_post')
systemhook(`sync_pre')
systemhook(`sync_post')
systemhook(`sync_all_pre')
systemhook(`sync_all_post')
systemhook(`fetch_all_pre')
systemhook(`fetch_all_post')
systemhook(`fetch_all_all_pre')
systemhook(`fetch_all_all_post')

install-data-local :
	install -d $(DESTDIR)/$(libexecdir)/paludis/hooks/clean_all_pre/
	install -d $(DESTDIR)/$(libexecdir)/paludis/hooks/clean_all_post/
	install -d $(DESTDIR)/$(libexecdir)/paludis/hooks/clean_pre/
	install -d $(DESTDIR)/$(libexecdir)/paludis/hooks/clean_post/
	install -d $(DESTDIR)/$(libexecdir)/paludis/hooks/uninstall_all_pre/
	install -d $(DESTDIR)/$(libexecdir)/paludis/hooks/uninstall_all_post/
	install -d $(DESTDIR)/$(libexecdir)/paludis/hooks/uninstall_pre/
	install -d $(DESTDIR)/$(libexecdir)/paludis/hooks/uninstall_post/
	install -d $(DESTDIR)/$(libexecdir)/paludis/hooks/install_pre/
	install -d $(DESTDIR)/$(libexecdir)/paludis/hooks/install_post/
	install -d $(DESTDIR)/$(libexecdir)/paludis/hooks/install_all_pre/
	install -d $(DESTDIR)/$(libexecdir)/paludis/hooks/install_all_post/
	install -d $(DESTDIR)/$(libexecdir)/paludis/hooks/install_pretend_pre/
	install -d $(DESTDIR)/$(libexecdir)/paludis/hooks/install_pretend_post/
	install -d $(DESTDIR)/$(libexecdir)/paludis/hooks/sync_pre/
	install -d $(DESTDIR)/$(libexecdir)/paludis/hooks/sync_post/
	install -d $(DESTDIR)/$(libexecdir)/paludis/hooks/sync_all_pre/
	install -d $(DESTDIR)/$(libexecdir)/paludis/hooks/sync_all_post/
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
	ln -sf ../common/log.bash $(DESTDIR)/$(libexecdir)/paludis/hooks/clean_all_pre/
	ln -sf ../common/log.bash $(DESTDIR)/$(libexecdir)/paludis/hooks/clean_all_post/
	ln -sf ../common/log.bash $(DESTDIR)/$(libexecdir)/paludis/hooks/clean_pre/
	ln -sf ../common/log.bash $(DESTDIR)/$(libexecdir)/paludis/hooks/clean_post/
	ln -sf ../common/log.bash $(DESTDIR)/$(libexecdir)/paludis/hooks/uninstall_all_pre/
	ln -sf ../common/log.bash $(DESTDIR)/$(libexecdir)/paludis/hooks/uninstall_all_post/
	ln -sf ../common/log.bash $(DESTDIR)/$(libexecdir)/paludis/hooks/uninstall_pre/
	ln -sf ../common/log.bash $(DESTDIR)/$(libexecdir)/paludis/hooks/uninstall_post/
	ln -sf ../common/log.bash $(DESTDIR)/$(libexecdir)/paludis/hooks/sync_pre/
	ln -sf ../common/log.bash $(DESTDIR)/$(libexecdir)/paludis/hooks/sync_post/
	ln -sf ../common/installable_cache_regen.bash $(DESTDIR)/$(libexecdir)/paludis/hooks/sync_all_post/
	ln -sf ../common/installed_cache_regen.bash $(DESTDIR)/$(libexecdir)/paludis/hooks/install_post/
	ln -sf ../common/installed_cache_regen.bash $(DESTDIR)/$(libexecdir)/paludis/hooks/uninstall_post/
	ln -sf ../common/installed_cache_regen.bash $(DESTDIR)/$(libexecdir)/paludis/hooks/clean_post/

uninstall-local :
	rm -f $(DESTDIR)/$(libexecdir)/paludis/hooks/*/gnu_info_index.bash
	rm -f $(DESTDIR)/$(libexecdir)/paludis/hooks/*/eselect_env_update.bash
	rm -f $(DESTDIR)/$(libexecdir)/paludis/hooks/*/log.bash

Makefile.am : Makefile.am.m4
	$(top_srcdir)/misc/do_m4.bash Makefile.am

all-local :
	chmod +x $(builddir)/news.hook $(builddir)/eselect_env_update.bash

