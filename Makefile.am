CLEANFILES = *~ done-check gmon.out *.gcov *.gcno *.gcda \
	automake-deps-dist-hack.tmp automake-deps-built-hack.tmp
MAINTAINERCLEANFILES = Makefile.in configure config/* aclocal.m4 \
			config.h config.h.in INSTALL COPYING
AUTOMAKE_OPTIONS = dist-bzip2 no-dist-gzip std-options
EXTRA_DIST = autogen.bash automake-deps-dist-hack.tmp
SUBDIRS = misc test paludis cran ebuild ruby src doc hooks eselect bash-completion

DISTCHECK_CONFIGURE_FLAGS = --enable-qa --enable-ruby --enable-glsa \
	--with-ruby-install-dir='$${DESTDIR}$${prefix}/ruby_dir'

automake-deps-built-hack.tmp : built-sources-subdirs
	touch $@

BUILT_SOURCES = automake-deps-built-hack.tmp

automake-deps-dist-hack.tmp :
	$(MAKE) -C paludis/util
	$(MAKE) -C paludis/args
	$(MAKE) -C paludis/qa message-sr.hh
	$(MAKE) -C src libcolour.a
	$(MAKE) -C src/paludis paludis.1
	$(MAKE) -C src/qualudis qualudis.1
	touch $@

dist-hook :
	rm $(distdir)/automake-deps-dist-hack.tmp

doxygen rdoc homepage htmlpages upload-homepage :
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

