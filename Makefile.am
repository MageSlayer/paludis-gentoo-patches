CLEANFILES = *~ done-check gmon.out *.gcov *.gcno *.gcda automake-deps-hack.tmp
MAINTAINERCLEANFILES = Makefile.in configure config/* aclocal.m4 \
			config.h config.h.in INSTALL COPYING
AUTOMAKE_OPTIONS = dist-bzip2 no-dist-gzip std-options
EXTRA_DIST = autogen.bash automake-deps-hack.tmp
SUBDIRS = misc test paludis cran ebuild ruby src doc hooks eselect

DISTCHECK_CONFIGURE_FLAGS = --enable-qa --enable-ruby --enable-glsa --with-ruby-install-dir='$${prefix}/ruby_dir'

automake-deps-hack.tmp :
	$(MAKE) -C paludis/util
	$(MAKE) -C paludis/args
	$(MAKE) -C paludis/qa message-sr.hh
	$(MAKE) -C src libcolour.a
	$(MAKE) -C src/paludis paludis.1
	$(MAKE) -C src/qualudis qualudis.1
	touch $@

dist-hook :
	rm $(distdir)/automake-deps-hack.tmp

doxygen :
	$(MAKE) -C doc doxygen

upload-homepage :
	$(MAKE) -C doc upload-homepage

check-local :
	touch $(top_builddir)/done-check

install-data-local :
	@[ -f $(top_builddir)/done-check ] || echo || true
	@[ -f $(top_builddir)/done-check ] || echo "**************************************" || true
	@[ -f $(top_builddir)/done-check ] || echo "Looks like you didn't run 'make check'" || true
	@[ -f $(top_builddir)/done-check ] || echo "      No cookies for you today.       " || true
	@[ -f $(top_builddir)/done-check ] || echo "**************************************" || true
	@[ -f $(top_builddir)/done-check ] || echo || true

