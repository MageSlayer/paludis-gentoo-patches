MAINTAINERCLEANFILES = Makefile.in
CLEANFILES = *~
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

install-data-local :
	install -d $(DESTDIR)/$(libexecdir)/paludis/hooks/uninstall_all_post
	ln -sf ../common/gnu_info_index.bash $(DESTDIR)/$(libexecdir)/paludis/hooks/uninstall_all_post/
	ln -sf ../common/eselect_env_update.bash $(DESTDIR)/$(libexecdir)/paludis/hooks/uninstall_all_post/
	install -d $(DESTDIR)/$(libexecdir)/paludis/hooks/install_all_post
	ln -sf ../common/gnu_info_index.bash $(DESTDIR)/$(libexecdir)/paludis/hooks/install_all_post/
	install -d $(DESTDIR)/$(libexecdir)/paludis/hooks/install_post
	ln -sf ../common/eselect_env_update.bash $(DESTDIR)/$(libexecdir)/paludis/hooks/install_post/
	for d in install install_all fetch fetch_all uninstall uninstall_all \
		sync sync_all ; do \
		install -d $(DESTDIR)/$(datadir)/paludis/hooks/$${d}_pre ; \
		touch      $(DESTDIR)/$(datadir)/paludis/hooks/$${d}_pre/.keep ; \
		install -d $(DESTDIR)/$(datadir)/paludis/hooks/$${d}_post ; \
		touch      $(DESTDIR)/$(datadir)/paludis/hooks/$${d}_post/.keep ; \
	done
	for d in metadata init fetch merge unmerge tidyup strip unpack compile \
		install test setup config nofetch preinst postinst prerm posrtm ; do \
		install -d $(DESTDIR)/$(datadir)/paludis/hooks/ebuild_$${d}_pre ; \
		touch      $(DESTDIR)/$(datadir)/paludis/hooks/ebuild_$${d}_pre/.keep ; \
		install -d $(DESTDIR)/$(datadir)/paludis/hooks/ebuild_$${d}_post ; \
		touch      $(DESTDIR)/$(datadir)/paludis/hooks/ebuild_$${d}_post/.keep ; \
	done

