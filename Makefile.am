CLEANFILES = *~ done-check gmon.out *.gcov *.gcno  *.gcda *.loT \
	automake-deps-dist-hack.tmp automake-deps-built-hack.tmp *.epicfail
MAINTAINERCLEANFILES = Makefile.in configure config/* aclocal.m4 \
			config.h config.h.in INSTALL
AUTOMAKE_OPTIONS = dist-bzip2 no-dist-gzip std-options foreign
EXTRA_DIST = autogen.bash automake-deps-dist-hack.tmp
SUBDIRS = . misc test paludis python ruby src doc hooks vim bash-completion zsh-completion pkg-config

DISTCHECK_CONFIGURE_FLAGS = --enable-ruby --enable-ruby-doc --enable-vim \
	--enable-python --enable-python-doc --enable-xml \
	--with-ruby-install-dir='$${DESTDIR}$${prefix}/ruby_dir' \
	--with-hacked-log-dir='$${DESTDIR}$${prefix}/log_dir' \
	--with-repositories=default,accounts \
	--with-environments=default,portage \
	--with-clients=default,accerso,appareo,cave,inquisitio,instruo \
	--with-default-distribution=giant-space-monkey \
	--enable-htmltidy

automake-deps-built-hack.tmp : built-sources-subdirs
	touch $@

BUILT_SOURCES = automake-deps-built-hack.tmp

automake-deps-dist-hack.tmp : all
	touch $@

dist-hook :
	rm $(distdir)/automake-deps-dist-hack.tmp

doxygen : built-sources
	$(MAKE) -C doc $@

epydoc : all
	$(MAKE) -C doc $@

htmldocs :
	$(MAKE) -C doc $@

documentation : doxygen epydoc htmldocs
	$(MAKE) -C doc $@

check-local : all
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
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

all-then-check :
	$(MAKE) all
	$(MAKE) check

