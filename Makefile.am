CLEANFILES = *~ done-check gmon.out *.gcov *.gcno *.gcda
MAINTAINERCLEANFILES = Makefile.in configure config/* aclocal.m4 \
			config.h config.h.in INSTALL COPYING
AUTOMAKE_OPTIONS = dist-bzip2 no-dist-gzip std-options
EXTRA_DIST = autogen.bash
SUBDIRS = misc test paludis ebuild src doc

doxygen :
	$(MAKE) -C doc doxygen

check-local :
	touch $(top_builddir)/done-check

install-data-local :
	@[ -f $(top_builddir)/done-check ] || echo || true
	@[ -f $(top_builddir)/done-check ] || echo "**************************************" || true
	@[ -f $(top_builddir)/done-check ] || echo "Looks like you didn't run 'make check'" || true
	@[ -f $(top_builddir)/done-check ] || echo "      No cookies for you today.       " || true
	@[ -f $(top_builddir)/done-check ] || echo "**************************************" || true
	@[ -f $(top_builddir)/done-check ] || echo || true

