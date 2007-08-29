CLEANFILES = *~ done-check gmon.out *.gcov *.gcno *.gcda \
	automake-deps-dist-hack.tmp automake-deps-built-hack.tmp
MAINTAINERCLEANFILES = Makefile.in configure config/* aclocal.m4 \
			config.h config.h.in INSTALL COPYING
AUTOMAKE_OPTIONS = dist-bzip2 no-dist-gzip std-options
EXTRA_DIST = autogen.bash automake-deps-dist-hack.tmp
SUBDIRS = misc test paludis python ruby src doc hooks eselect vim bash-completion zsh-completion pkg-config

DISTCHECK_CONFIGURE_FLAGS = --enable-qa --enable-ruby --enable-glsa --enable-vim --enable-python \
	--with-ruby-install-dir='$${DESTDIR}$${prefix}/ruby_dir' \
	--with-repositories=default \
	--with-environments=default,portage \
	--with-clients=default,accerso,contrarius,inquisitio \
	--with-default-distribution=giant-space-monkey

automake-deps-built-hack.tmp : built-sources-subdirs
	touch $@

BUILT_SOURCES = automake-deps-built-hack.tmp

automake-deps-dist-hack.tmp : built-sources-subdirs
	$(MAKE) -C paludis/util
	$(MAKE) -C paludis/args
	$(MAKE) -C paludis/ libpaludismanpagethings.la
	$(MAKE) -C src/output liboutput.a
	$(MAKE) -C src/common_args libcommonargs.a
	$(MAKE) -C src/clients/paludis paludis.1 paludis.html
	$(MAKE) -C src/clients/qualudis qualudis.1 qualudis.html
	$(MAKE) -C src/clients/contrarius contrarius.1 contrarius.html
	$(MAKE) -C src/clients/adjutrix adjutrix.1 adjutrix.html
	$(MAKE) -C src/clients/inquisitio inquisitio.1 inquisitio.html
	$(MAKE) -C src/clients/accerso accerso.1 accerso.html
	touch $@

dist-hook :
	rm $(distdir)/automake-deps-dist-hack.tmp

doxygen rdoc epydoc homepage htmlpages upload-homepage :
	$(MAKE) -C doc $@

check-local :
	touch $(top_builddir)/done-check

install-data-local :
	@[ -f $(top_builddir)/done-check ] || echo || true
	@[ -f $(top_builddir)/done-check ] || echo "**************************************" || true
	@[ -f $(top_builddir)/done-check ] || echo "Looks like you didn't run 'make check'" || true
	@[ -f $(top_builddir)/done-check ] || echo "      No cookies for you today.       " || true
	@[ -f $(top_builddir)/done-check ] || echo "**************************************" || true
	@[ -f $(top_builddir)/done-check ] || echo || true

built-sources : $(BUILT_SOURCES) built-sources-subdirs

built-sources-subdirs :
	for s in $(SUBDIRS) ; do $(MAKE) -C $$s built-sources || exit 1 ; done

